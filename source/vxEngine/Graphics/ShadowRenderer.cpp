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
#include "../EngineConfig.h"
#include <vxLib/gl/gl.h>
#include <string>
#include "Segment.h"
#include "Commands.h"
#include <vxLib/gl/ShaderManager.h>
#include <vxLib/gl/ProgramPipeline.h>
#include <vxLib/gl/Buffer.h>
#include "../GpuStructs.h"
#include "../gl/BufferBindingManager.h"
#include "Commands/ProgramUniformCommand.h"

namespace Graphics
{
	ShadowRenderer::ShadowRenderer()
		:m_textureCount(0)
	{

	}

	ShadowRenderer::~ShadowRenderer()
	{

	}

	void ShadowRenderer::createShadowTextureBuffer()
	{
		UniformShadowTextureBufferBlock data;
		memset(&data, 0, sizeof(UniformShadowTextureBufferBlock));

		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
		desc.flags = vx::gl::BufferStorageFlags::Write;
		desc.immutable = 1;
		desc.pData = &data;
		desc.size = sizeof(UniformShadowTextureBufferBlock);

		auto sid = s_objectManager->createBuffer("uniformShadowTextureBuffer", desc);
		VX_ASSERT(sid.value != 0);
	}

	void ShadowRenderer::createShadowTextures()
	{
		m_textureCount = s_settings->m_maxActiveLights;
		auto textureResolution = s_settings->m_shadowMapResolution;

		auto shadowTexBuffer = s_objectManager->getBuffer("uniformShadowTextureBuffer");

		m_shadowDepthTextureIds = vx::make_unique<u32[]>(m_textureCount);
		m_shadowDiffuseTextureIds = vx::make_unique<u32[]>(m_textureCount);

		vx::gl::TextureDescription depthDesc;
		depthDesc.format = vx::gl::TextureFormat::DEPTH32F;
		depthDesc.type = vx::gl::TextureType::Texture_Cubemap;
		depthDesc.size = vx::ushort3(textureResolution, textureResolution, 6);
		depthDesc.miplevels = 1;
		depthDesc.sparse = 0;

		vx::gl::TextureDescription diffuseDesc;
		diffuseDesc.format = vx::gl::TextureFormat::RGB16F;
		diffuseDesc.type = vx::gl::TextureType::Texture_Cubemap;
		diffuseDesc.size = vx::ushort3(textureResolution, textureResolution, 6);
		diffuseDesc.miplevels = 1;
		diffuseDesc.sparse = 0;

		const char textureDepthName[] = "shadowDepthTexture";
		const char textureDiffuseName[] = "shadowDiffuseTexture";
		const auto textureDepthNameSize = sizeof(textureDepthName);
		const auto textureDiffuseNameSize = sizeof(textureDiffuseName);

		char depthNameBuffer[24];
		char diffuseNameBuffer[24];

		memcpy(depthNameBuffer, textureDepthName, textureDepthNameSize);
		memcpy(diffuseNameBuffer, textureDiffuseName, textureDiffuseNameSize);

		auto mappedBuffer = shadowTexBuffer->map<UniformShadowTextureBufferBlock>(vx::gl::Map::Write_Only);

		for (u32 i = 0; i < m_textureCount; ++i)
		{
			sprintf(depthNameBuffer + textureDepthNameSize - 1, "%u", i);
			sprintf(diffuseNameBuffer + textureDiffuseNameSize - 1, "%u", i);

			auto depthTextureSid = s_objectManager->createTexture(depthNameBuffer, depthDesc);
			auto diffuseTextureSid = s_objectManager->createTexture(diffuseNameBuffer, diffuseDesc);
			VX_ASSERT(depthTextureSid.value != 0u);
			VX_ASSERT(diffuseTextureSid.value != 0u);

			auto depthTexture = s_objectManager->getTexture(depthTextureSid);
			auto diffuseTexture = s_objectManager->getTexture(diffuseTextureSid);

			m_shadowDepthTextureIds[i] = depthTexture->getId();
			m_shadowDiffuseTextureIds[i] = diffuseTexture->getId();

			depthTexture->setFilter(vx::gl::TextureFilter::LINEAR, vx::gl::TextureFilter::LINEAR);
			depthTexture->setWrapMode3D(vx::gl::TextureWrapMode::CLAMP_TO_EDGE, vx::gl::TextureWrapMode::CLAMP_TO_EDGE, vx::gl::TextureWrapMode::CLAMP_TO_EDGE);

			diffuseTexture->setFilter(vx::gl::TextureFilter::LINEAR, vx::gl::TextureFilter::LINEAR);
			diffuseTexture->setWrapMode3D(vx::gl::TextureWrapMode::CLAMP_TO_EDGE, vx::gl::TextureWrapMode::CLAMP_TO_EDGE, vx::gl::TextureWrapMode::CLAMP_TO_EDGE);

			glTextureParameteri(depthTexture->getId(), GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTextureParameteri(depthTexture->getId(), GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
			//float ones[] = { 0, 0, 0, 0 };
			//glTextureParameterfv(m_pColdData->m_shadowTexture.getId(), GL_TEXTURE_BORDER_COLOR, ones);

			depthTexture->makeTextureResident();

			mappedBuffer->u_shadowTextures[i] = depthTexture->getTextureHandle();
		}
	}

	void ShadowRenderer::createFramebuffer()
	{
		auto sid = s_objectManager->createFramebuffer("shadowFbo");
		auto fbo = s_objectManager->getFramebuffer(sid);

		//fbo->attachTexture(vx::gl::Attachment::Color0, m_shadowDiffuseTextureIds[0], 0);
		//fbo->attachTexture(vx::gl::Attachment::Depth, m_shadowDepthTextureIds[0], 0);

		glNamedFramebufferDrawBuffer(fbo->getId(), GL_NONE);

		//auto status = fbo->checkStatus();
		//VX_ASSERT(status == GL_FRAMEBUFFER_COMPLETE);
	}

	void ShadowRenderer::createLightDrawCommandBuffers()
	{
		auto cmdCount = s_settings->m_maxMeshInstances * s_settings->m_maxActiveLights;
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
		m_lightCmdBufferSid = sid;
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
		drawCmd.set(GL_POINTS, 0, s_settings->m_maxMeshInstances * s_settings->m_maxActiveLights);
		segment.pushCommand(drawCmd);

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

		auto maxMeshInstances = s_settings->m_maxMeshInstances;

		Graphics::BarrierCommand barrierCmd;
		barrierCmd.set(GL_SHADER_STORAGE_BARRIER_BIT);
		segment.pushCommand(barrierCmd);

		Graphics::MultiDrawElementsIndirectCountCommand drawCmd;
		drawCmd.set(GL_TRIANGLES, GL_UNSIGNED_INT, maxMeshInstances);
		segment.pushCommand(drawCmd);

		return segment;
	}

	void ShadowRenderer::initialize()
	{
		createShadowTextureBuffer();
		createShadowTextures();
		createFramebuffer();

		createLightDrawCommandBuffers();
	}

	void ShadowRenderer::update()
	{

	}

	void ShadowRenderer::updateDrawCmd(const vx::gl::DrawElementsIndirectCommand &cmd, u32 index)
	{
		auto cmdBuffer = s_objectManager->getBuffer(m_lightCmdBufferSid);

		auto offset = sizeof(vx::gl::DrawElementsIndirectCommand) * index;
		auto offsetPerLight = sizeof(vx::gl::DrawElementsIndirectCommand) * s_settings->m_maxMeshInstances;

		for (u32 i = 0; i < s_settings->m_maxActiveLights; ++i)
		{
			auto mappedCmd = cmdBuffer->mapRange<vx::gl::DrawElementsIndirectCommand>(offset, sizeof(vx::gl::DrawElementsIndirectCommand), vx::gl::MapRange::Write);
			*mappedCmd = cmd;

			offset += offsetPerLight;
		}
	}

	void ShadowRenderer::updateDrawCmds()
	{
		auto meshCmdBuffer = s_objectManager->getBuffer("meshCmdBuffer");

		auto meshCmds = meshCmdBuffer->map<vx::gl::DrawElementsIndirectCommand>(vx::gl::Map::Read_Only);
		auto lightCmdBuffer = s_objectManager->getBuffer(m_lightCmdBufferSid);

		auto sizeInBytes = sizeof(vx::gl::DrawElementsIndirectCommand) * s_settings->m_maxMeshInstances;
		u32 offset = 0;
		for (u32 i = 0; i < s_settings->m_maxActiveLights; ++i)
		{
			auto mappedCmd = lightCmdBuffer->mapRange<vx::gl::DrawElementsIndirectCommand>(offset, sizeof(vx::gl::DrawElementsIndirectCommand), vx::gl::MapRange::Write);
			memcpy(mappedCmd.get(), meshCmds.get(), sizeInBytes);
			mappedCmd.unmap();

			offset += sizeInBytes;
		}
	}

	void ShadowRenderer::getSegments(std::vector<std::pair<std::string, Segment>>* segments)
	{
		auto segmentResetLightCmdBuffer = createSegmentResetCmdBuffer();
		segments->push_back(std::make_pair(std::string("segmentResetLightCmdBuffer"),segmentResetLightCmdBuffer));

		auto segmentCullMeshes = createSegmentCullMeshes();
		segments->push_back(std::make_pair(std::string("segmentLightCullMeshes"), segmentCullMeshes));

		auto maxMeshInstances = s_settings->m_maxMeshInstances;
		auto resolution = s_settings->m_shadowMapResolution;

		auto lightCmdBuffer = s_objectManager->getBuffer(m_lightCmdBufferSid);

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
		stateDesc.indirectBuffer = lightCmdBuffer->getId();
		stateDesc.paramBuffer = meshParamBuffer->getId();
		stateDesc.pipeline = pipeline->getId();
		stateDesc.polygonOffsetFillState = true;

		Graphics::State state;
		state.set(stateDesc);

		Graphics::ViewportCommand viewportCmd;
		viewportCmd.set(vx::uint2(0), vx::uint2(resolution, resolution));

		Graphics::PolygonOffsetCommand polyCmd;
		polyCmd.set(2.5f, 10.0f);

		Graphics::Segment segmentCreateShadowmap;
		segmentCreateShadowmap.setState(state);

		segmentCreateShadowmap.pushCommand(viewportCmd);
		segmentCreateShadowmap.pushCommand(polyCmd);

		Graphics::BarrierCommand barrierCmd;
		barrierCmd.set(GL_COMMAND_BARRIER_BIT);
		segmentCreateShadowmap.pushCommand(barrierCmd);

		Graphics::ProgramUniformCommand uniformCmd;
		uniformCmd.set(gsShader, 0, 1, vx::gl::DataType::Unsigned_Int);

		auto cmdSizeInBytes = sizeof(vx::gl::DrawElementsIndirectCommand) * s_settings->m_maxMeshInstances;
		u32 cmdOffset = 0;

		auto maxLightCount = s_settings->m_maxActiveLights;
		for (u32 i = 0; i < maxLightCount; ++i)
		{
			const auto ll = GL_UNSIGNED_INT;
			Graphics::FramebufferTextureCommand fbDepthTexCmd;
			fbDepthTexCmd.set(fbo->getId(), GL_DEPTH_ATTACHMENT, m_shadowDepthTextureIds[i], 0);

			Graphics::ClearCommand clearCmd;
			clearCmd.set(GL_DEPTH_BUFFER_BIT);

			Graphics::MultiDrawElementsIndirectCountCommand drawCmd;
			drawCmd.set(GL_TRIANGLES, GL_UNSIGNED_INT, maxMeshInstances, cmdOffset);

			segmentCreateShadowmap.pushCommand(fbDepthTexCmd);
			segmentCreateShadowmap.pushCommand(clearCmd);
			segmentCreateShadowmap.pushCommand(uniformCmd, i);
			segmentCreateShadowmap.pushCommand(drawCmd);

			cmdOffset += cmdSizeInBytes;
		}

		segments->push_back(std::make_pair(std::string("segmentCreateShadowmap"),segmentCreateShadowmap));
	}

	void ShadowRenderer::clearData()
	{
	}

	void ShadowRenderer::bindBuffers()
	{
		auto shadowTexBuffer = s_objectManager->getBuffer("uniformShadowTextureBuffer");
		gl::BufferBindingManager::bindBaseUniform(9, shadowTexBuffer->getId());
	}

	const u32* ShadowRenderer::getTextureIds() const
	{
		return m_shadowDepthTextureIds.get();
	}
}