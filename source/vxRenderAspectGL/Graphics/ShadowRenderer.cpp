/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "ShadowRenderer.h"
#include "../gl/ObjectManager.h"
#include <vxGL/gl.h>
#include <string>
#include "Segment.h"
#include "Commands.h"
#include <vxGL/ShaderManager.h>
#include <vxGL/ProgramPipeline.h>
#include <vxGL/Buffer.h>
#include "../GpuStructs.h"
#include "../gl/BufferBindingManager.h"
#include "Commands/ProgramUniformCommand.h"
#include "CommandList.h"
#include "Commands/CullFaceCommand.h"
#include "Commands/DepthRangeCommand.h"
#include <vxEngineLib/EngineConfig.h>
#include <vxLib/File/FileHandle.h>
#include <vxEngineLib/Light.h>
#include <vxLib/Graphics/Camera.h>
#include <vxGL/Texture.h>
#include <vxGL/Framebuffer.h>
#include <vxGL/VertexArray.h>
#include "../Frustum.h"

namespace ShadowRendererCpp
{
	void getShadowTransform(const Gpu::LightData &light, ShadowTransform* shadowTransform)
	{
		auto n = 0.1f;
		auto f = light.falloff;

		auto lightPos = light.position;
		auto projectionMatrix = vx::MatrixPerspectiveFovRHDX(vx::degToRad(90.0f), 1.0f, n, f);

		vx::mat4 viewMatrices[6];
		// X+
		vx::float4 up = { 0, -1, 0, 0 };
		vx::float4 dir = { 1, 0, 0, 0 };
		viewMatrices[0] = vx::MatrixLookToRH(lightPos, vx::loadFloat4(dir), vx::loadFloat4(up));
		// X-
		up = { 0, -1, 0, 0 };
		dir = { -1, 0, 0, 0 };
		viewMatrices[1] = vx::MatrixLookToRH(lightPos, vx::loadFloat4(dir), vx::loadFloat4(up));
		// Y+
		up = { 0, 0, 1, 0 };
		dir = vx::float4(0, 1, 0, 0);
		viewMatrices[2] = vx::MatrixLookToRH(lightPos, vx::loadFloat4(dir), vx::loadFloat4(up));
		// Y-
		up = { 0, 0, -1, 0 };
		dir = vx::float4(0, -1, 0, 0);
		viewMatrices[3] = vx::MatrixLookToRH(lightPos, vx::loadFloat4(dir), vx::loadFloat4(up));
		// Z+
		up = { 0, -1, 0, 0 };
		dir = vx::float4(0, 0, 1, 0);
		viewMatrices[4] = vx::MatrixLookToRH(lightPos, vx::loadFloat4(dir), vx::loadFloat4(up));
		// Z-
		up = { 0, -1, 0, 0 };
		dir = vx::float4(0, 0, -1, 0);
		viewMatrices[5] = vx::MatrixLookToRH(lightPos, vx::loadFloat4(dir), vx::loadFloat4(up));

		/*f32 xmax = lightPos.f[0] + light.falloff;
		f32 xmin = lightPos.f[0] - light.falloff;

		f32 ymax = lightPos.f[1] + light.falloff;
		f32 ymin = lightPos.f[1] - light.falloff;

		auto sx = 2 / (xmax - xmin);
		auto sy = 2 / (ymax - ymin);
		auto ox = -(sx * (xmax + xmin)) / 2.0f;
		auto oy = -(sy * (ymax + ymin)) / 2.0f;

		vx::mat4 scaleMatrix;
		scaleMatrix.c[0] = { sx, 0, 0, 0 };
		scaleMatrix.c[1] = { 0, sy, 0, 0 };
		scaleMatrix.c[2] = { 0, 0, 1, 0 };
		scaleMatrix.c[3] = { ox, oy, 0, 1 };

		shadowTransform->scaleMatrix = scaleMatrix;*/
		shadowTransform->projectionMatrix = projectionMatrix;
		for (u32 i = 0; i < 6; ++i)
		{
			shadowTransform->viewMatrix[i] = viewMatrices[i];
			shadowTransform->pvMatrix[i] = projectionMatrix * viewMatrices[i];
		}
	}
}

namespace Graphics
{
	ShadowRenderer::ShadowRenderer()
		:m_lights(),
		m_shadowDepthTextureIds(),
		m_lightCmdBufferSid(),
		m_lightCount(0),
		m_maxShadowLights(0)
	{

	}

	ShadowRenderer::~ShadowRenderer()
	{

	}

	void ShadowRenderer::createShadowTextureBuffer()
	{
		auto maxShadowLightCount = m_maxShadowLights;

		auto data = vx::make_unique<u64[]>(maxShadowLightCount);
		auto sizeInBytes = sizeof(u64) * maxShadowLightCount;
		memset(data.get(), 0, sizeInBytes);

		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
		desc.flags = vx::gl::BufferStorageFlags::Write;
		desc.immutable = 1;
		desc.pData = &data;
		desc.size = sizeInBytes;

		auto sid = s_objectManager->createBuffer("uniformShadowTextureBuffer", desc);
		VX_ASSERT(sid.value != 0);
	}

	void ShadowRenderer::createShadowTextures()
	{
		auto maxShadowLightCount = m_maxShadowLights;
		auto shadowMapResolution = s_settings->m_rendererSettings.m_shadowSettings.m_shadowMapResolution;

		auto shadowTexBuffer = s_objectManager->getBuffer("uniformShadowTextureBuffer");

		m_shadowDepthTextureIds = vx::make_unique<u32[]>(maxShadowLightCount);

		vx::gl::TextureDescription depthDesc;
		depthDesc.format = vx::gl::TextureFormat::DEPTH32F;
		depthDesc.type = vx::gl::TextureType::Texture_Cubemap;
		depthDesc.size = vx::ushort3(shadowMapResolution, shadowMapResolution, 6);
		depthDesc.miplevels = 1;
		depthDesc.sparse = 0;

		const char textureDepthName[] = "shadowDepthTexture";
		const auto textureDepthNameSize = sizeof(textureDepthName);

		char depthNameBuffer[32];

		memcpy(depthNameBuffer, textureDepthName, textureDepthNameSize);

		auto mappedBuffer = shadowTexBuffer->map<u64>(vx::gl::Map::Write_Only);

		for (u32 i = 0; i < maxShadowLightCount; ++i)
		{
			sprintf(depthNameBuffer + textureDepthNameSize - 1, "%u", i);
			auto depthTextureSid = s_objectManager->createTexture(depthNameBuffer, depthDesc);
			VX_ASSERT(depthTextureSid.value != 0u);

			auto depthTexture = s_objectManager->getTexture(depthTextureSid);

			m_shadowDepthTextureIds[i] = depthTexture->getId();

			depthTexture->setFilter(vx::gl::TextureFilter::LINEAR, vx::gl::TextureFilter::LINEAR);
			depthTexture->setWrapMode3D(vx::gl::TextureWrapMode::CLAMP_TO_EDGE, vx::gl::TextureWrapMode::CLAMP_TO_EDGE, vx::gl::TextureWrapMode::CLAMP_TO_EDGE);

			glTextureParameteri(depthTexture->getId(), GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTextureParameteri(depthTexture->getId(), GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

			depthTexture->makeTextureResident();

			mappedBuffer[i] = depthTexture->getTextureHandle();
		}
	}

	void ShadowRenderer::createFramebuffer()
	{
		auto sid = s_objectManager->createFramebuffer("shadowFbo");
		auto fbo = s_objectManager->getFramebuffer(sid);

		glNamedFramebufferDrawBuffer(fbo->getId(), GL_NONE);

		//auto status = fbo->checkStatus();
		//VX_ASSERT(status == GL_FRAMEBUFFER_COMPLETE);
	}

	void ShadowRenderer::createLightDrawCommandBuffers()
	{
		/*auto cmdCount = m_maxMeshInstanceCount * m_maxShadowLights;
		auto commands = vx::make_unique<vx::gl::DrawElementsIndirectCommand[]>(cmdCount);
		for (u32 i = 0; i < cmdCount; ++i)
		{
			commands[i].firstIndex = 0;
			commands[i].count = 0;
		}

		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Draw_Indirect_Buffer;
		desc.flags = vx::gl::BufferStorageFlags::Write;
		desc.immutable = 1;
		desc.pData = commands.get();
		desc.size = sizeof(vx::gl::DrawElementsIndirectCommand) * cmdCount;

		auto sid = s_objectManager->createBuffer("lightCmdBuffer", desc);
		VX_ASSERT(sid.value != 0u);
		m_lightCmdBufferSid = sid;*/
	}

	Segment ShadowRenderer::createSegmentResetCmdBuffer() const
	{
		auto vao = s_objectManager->getVertexArray("emptyVao");
		auto pipeline = s_shaderManager->getPipeline("resetLightCmdBuffer.pipe");

		Graphics::StateDescription stateDesc;
		stateDesc.fbo = 0;
		stateDesc.vao = vao->getId();
		stateDesc.blendState = false;
		stateDesc.depthState = true;
		stateDesc.indirectBuffer = 0;
		stateDesc.paramBuffer = 0;
		stateDesc.pipeline = pipeline->getId();
		stateDesc.polygonOffsetFillState = true;

		Graphics::State state;
		state.set(stateDesc);

		Segment segment;
		segment.setState(state);

		DrawArraysCommand drawCmd;
		//drawCmd.set(GL_POINTS, 0, m_maxMeshInstanceCount * m_maxShadowLights);
		//segment.pushCommand(drawCmd);

		return segment;
	}

	Segment ShadowRenderer::createSegmentCullMeshes() const
	{
		auto vao = s_objectManager->getVertexArray("meshVao");
		auto meshCmdBuffer = s_objectManager->getBuffer("meshCmdBuffer");
		auto paramBuffer = s_objectManager->getBuffer("meshParamBuffer");
		auto pipeline = s_shaderManager->getPipeline("createLightCmdBuffer.pipe");
		auto vsShader = pipeline->getVertexShader();

		Graphics::StateDescription stateDesc;
		stateDesc.fbo = 0;
		stateDesc.vao = vao->getId();
		stateDesc.blendState = false;
		stateDesc.depthState = true;
		stateDesc.indirectBuffer = meshCmdBuffer->getId();
		stateDesc.paramBuffer = paramBuffer->getId();
		stateDesc.pipeline = pipeline->getId();
		stateDesc.polygonOffsetFillState = false;

		Graphics::State state;
		state.set(stateDesc);

		Segment segment;
		segment.setState(state);

		/*auto maxMeshInstances = m_maxMeshInstanceCount;

		Graphics::BarrierCommand barrierCmd;
		barrierCmd.set(GL_SHADER_STORAGE_BARRIER_BIT);
		segment.pushCommand(barrierCmd);

		Graphics::MultiDrawElementsIndirectCountCommand drawCmd;
		drawCmd.set(GL_TRIANGLES, GL_UNSIGNED_INT, maxMeshInstances);
		segment.pushCommand(drawCmd);*/

		return segment;
	}

	void ShadowRenderer::initialize(vx::StackAllocator* scratchAllocator, const void*)
	{
		auto maxShadowLights = s_settings->m_rendererSettings.m_shadowSettings.m_maxShadowCastingLights;
		m_maxShadowLights = maxShadowLights;

		s_shaderManager->loadPipeline(vx::FileHandle("shadow.pipe"), "shadow.pipe", scratchAllocator);
		//s_shaderManager->loadPipeline(vx::FileHandle("resetLightCmdBuffer.pipe"), "resetLightCmdBuffer.pipe", scratchAllocator);
		//s_shaderManager->loadPipeline(vx::FileHandle("createLightCmdBuffer.pipe"), "createLightCmdBuffer.pipe", scratchAllocator);

		createShadowTextureBuffer();
		createShadowTextures();
		createFramebuffer();

		createLightDrawCommandBuffers();

		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
		desc.size = sizeof(ShadowTransform) * maxShadowLights + sizeof(__m128);
		desc.immutable = 1;
		desc.flags = vx::gl::BufferStorageFlags::Write;

		s_objectManager->createBuffer("ShadowTransformBuffer", desc);
	}

	void ShadowRenderer::shutdown()
	{

	}

	void ShadowRenderer::updateDrawCmd(const vx::gl::DrawElementsIndirectCommand &cmd, u32 index)
	{
		/*auto cmdBuffer = s_objectManager->getBuffer(m_lightCmdBufferSid);

		auto offset = sizeof(vx::gl::DrawElementsIndirectCommand) * index;
		auto offsetPerLight = sizeof(vx::gl::DrawElementsIndirectCommand) * m_maxMeshInstanceCount;

		for (u32 i = 0; i < m_maxShadowLights; ++i)
		{
			auto mappedCmd = cmdBuffer->mapRange<vx::gl::DrawElementsIndirectCommand>(offset, sizeof(vx::gl::DrawElementsIndirectCommand), vx::gl::MapRange::Write);
			*mappedCmd = cmd;

			offset += offsetPerLight;
		}*/
	}

	void ShadowRenderer::updateDrawCmds()
	{
		/*auto meshCmdBuffer = s_objectManager->getBuffer("meshCmdBuffer");

		auto meshCmds = meshCmdBuffer->map<vx::gl::DrawElementsIndirectCommand>(vx::gl::Map::Read_Only);
		auto lightCmdBuffer = s_objectManager->getBuffer(m_lightCmdBufferSid);

		auto sizeInBytes = sizeof(vx::gl::DrawElementsIndirectCommand) * m_maxMeshInstanceCount;
		u32 offset = 0;
		for (u32 i = 0; i < m_maxShadowLights; ++i)
		{
			auto mappedCmd = lightCmdBuffer->mapRange<vx::gl::DrawElementsIndirectCommand>(offset, sizeof(vx::gl::DrawElementsIndirectCommand), vx::gl::MapRange::Write);
			memcpy(mappedCmd.get(), meshCmds.get(), sizeInBytes);
			mappedCmd.unmap();

			offset += sizeInBytes;
		}*/
	}

	void ShadowRenderer::getCommandList(CommandList* cmdList)
	{
		//auto segmentResetLightCmdBuffer = createSegmentResetCmdBuffer();
		//auto segmentCullMeshes = createSegmentCullMeshes();

		//
		//m_maxMeshInstanceCount = s_settings->m_rendererSettings.m_maxMeshInstances;

		auto maxShadowLightCount = m_maxShadowLights;
		auto maxMeshInstances = s_settings->m_rendererSettings.m_maxMeshInstances;
		auto shadowMapResolution = s_settings->m_rendererSettings.m_shadowSettings.m_shadowMapResolution;

		//auto lightCmdBuffer = s_objectManager->getBuffer(m_lightCmdBufferSid);

		auto fbo = s_objectManager->getFramebuffer("shadowFbo");
		auto vao = s_objectManager->getVertexArray("meshVao");
		auto meshCmdBuffer = s_objectManager->getBuffer("meshCmdBuffer");
		auto meshParamBuffer = s_objectManager->getBuffer("meshParamBuffer");
		auto pipeline = s_shaderManager->getPipeline("shadow.pipe");
		auto gsShader = pipeline->getGeometryShader();

		Graphics::StateDescription stateDesc;
		stateDesc.fbo = fbo->getId();
		stateDesc.vao = vao->getId();
		stateDesc.blendState = false;
		stateDesc.depthState = true;
		stateDesc.indirectBuffer = meshCmdBuffer->getId();
		stateDesc.paramBuffer = meshParamBuffer->getId();
		stateDesc.pipeline = pipeline->getId();
		stateDesc.polygonOffsetFillState = true;

		Graphics::State state;
		state.set(stateDesc);

		Graphics::Segment segmentCreateShadowmap;
		segmentCreateShadowmap.setState(state);

		/*GpuProfilePushCommand gpuPushCmd;
		gpuPushCmd.set(s_gpuProfiler, "shadow");
		GpuProfilePopCommand gpuPopCmd;
		gpuPopCmd.set(s_gpuProfiler);

		CpuProfilePushCommand cpuPushCmd;
		cpuPushCmd.set("shadow");
		CpuProfilePopCommand cpuPopCmd;*/

		//segmentCreateShadowmap.pushCommand(cpuPushCmd);
		//segmentCreateShadowmap.pushCommand(gpuPushCmd);

		Graphics::ViewportCommand viewportCmd;
		viewportCmd.set(vx::uint2(0), vx::uint2(shadowMapResolution));
		segmentCreateShadowmap.pushCommand(viewportCmd);

		Graphics::CullFaceCommand cullFaceCommand;
		cullFaceCommand.set(GL_FRONT);
		segmentCreateShadowmap.pushCommand(cullFaceCommand);

		Graphics::BarrierCommand barrierCmd;
		barrierCmd.set(GL_COMMAND_BARRIER_BIT);
		segmentCreateShadowmap.pushCommand(barrierCmd);

		Graphics::ProgramUniformCommand uniformCmd;
		uniformCmd.set(gsShader, 0, 1, vx::gl::DataType::Unsigned_Int);

		//auto cmdSizeInBytes = sizeof(vx::gl::DrawElementsIndirectCommand) * m_maxMeshInstanceCount;
		//u32 cmdOffset = 0;

		for (u32 i = 0; i < maxShadowLightCount; ++i)
		{
			const auto ll = GL_UNSIGNED_INT;
			Graphics::FramebufferTextureCommand fbDepthTexCmd;
			fbDepthTexCmd.set(fbo->getId(), GL_DEPTH_ATTACHMENT, m_shadowDepthTextureIds[i], 0);

			Graphics::ClearCommand clearCmd;
			clearCmd.set(GL_DEPTH_BUFFER_BIT);

			Graphics::MultiDrawElementsIndirectCountCommand drawCmd;
			drawCmd.set(GL_TRIANGLES, GL_UNSIGNED_INT, maxMeshInstances, 0);

			segmentCreateShadowmap.pushCommand(fbDepthTexCmd);
			segmentCreateShadowmap.pushCommand(clearCmd);
			segmentCreateShadowmap.pushCommand(uniformCmd, i);
			segmentCreateShadowmap.pushCommand(drawCmd);

			//	cmdOffset += cmdSizeInBytes;
		}

		cullFaceCommand.set(GL_BACK);
		segmentCreateShadowmap.pushCommand(cullFaceCommand);
		//segmentCreateShadowmap.pushCommand(gpuPopCmd);
		//segmentCreateShadowmap.pushCommand(cpuPopCmd);

		//cmdList->pushSegment(segmentResetLightCmdBuffer, "segmentResetLightCmdBuffer");
		//cmdList->pushSegment(segmentCullMeshes, "segmentCullMeshes");
		cmdList->pushSegment(segmentCreateShadowmap, "segmentCreateShadowmap");
	}

	void ShadowRenderer::clearData()
	{
	}

	void ShadowRenderer::bindBuffers()
	{
		auto shadowTexBuffer = s_objectManager->getBuffer("uniformShadowTextureBuffer");
		auto pShadowTransformBuffer = s_objectManager->getBuffer("ShadowTransformBuffer");

		//auto lightCmdBuffer = s_objectManager->getBuffer("lightCmdBuffer");

		gl::BufferBindingManager::bindBaseUniform(5, pShadowTransformBuffer->getId());
		gl::BufferBindingManager::bindBaseUniform(9, shadowTexBuffer->getId());

		//gl::BufferBindingManager::bindBaseShaderStorage(5, lightCmdBuffer->getId());
	}

	void ShadowRenderer::setLights(const Light* lights, u32 count)
	{
		m_lights = vx::make_unique<ShadowTransform[]>(count);
		for (u32 i = 0;i < count; ++i)
		{
			auto &light = lights[i];

			Gpu::LightData data;
			data.position = vx::loadFloat3(light.m_position);
			data.falloff = light.m_falloff;
			data.lumen = light.m_lumen;

			ShadowTransform bufferData;
			bufferData.position = data.position;
			bufferData.falloff_lumen.x = data.falloff;
			bufferData.falloff_lumen.y = data.lumen;

			ShadowRendererCpp::getShadowTransform(data, &bufferData);

			m_lights[i] = bufferData;
		}
		m_lightCount = count;

		m_distances = vx::make_unique<std::pair<f32, u32>[]>(count);
	}

	void ShadowRenderer::cullLights(const Frustum &frustum, const vx::Camera &camera)
	{
		auto lightCount = m_lightCount;
		if (lightCount == 0)
			return;

		auto lights = m_lights.get();

		auto maxLights = m_maxShadowLights;

		auto lightDistances = m_distances.get();

		auto cameraPosition = camera.getPosition();

		for (u32 i = 0; i < lightCount; ++i)
		{
			auto lightPosition = lights[i].position;

			auto distance = _mm_sub_ps(cameraPosition, lightPosition);
			distance = vx::dot3(distance, distance);
			distance = _mm_sqrt_ps(distance);

			_mm_store_ss(&lightDistances[i].first, distance);
			lightDistances[i].second = i;
		}

		std::sort(lightDistances, lightDistances + lightCount, [](const std::pair<f32, u32> &lhs, const std::pair<f32, u32> &rhs)
		{
			return lhs.first < rhs.first;
		});


		auto activeLights = std::min(lightCount, maxLights);

		auto offset = maxLights * sizeof(ShadowTransform);
		auto dataSize = activeLights * sizeof(ShadowTransform);

		auto buffer = s_objectManager->getBuffer("ShadowTransformBuffer");
		auto mappedBuffer = buffer->mapRange<ShadowTransform>(0, dataSize, vx::gl::MapRange::Write);
		for (u32 i = 0; i < activeLights; ++i)
		{

			mappedBuffer[i] = lights[i];
		}
		mappedBuffer.unmap();

		auto mappedBufferCount = buffer->mapRange<u32>(offset, sizeof(u32), vx::gl::MapRange::Write);
		*mappedBufferCount = activeLights;
		mappedBufferCount.unmap();
	}

	const u32* ShadowRenderer::getTextureIds() const
	{
		return m_shadowDepthTextureIds.get();
	}
}