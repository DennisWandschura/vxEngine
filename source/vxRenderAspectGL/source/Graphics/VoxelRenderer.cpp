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
#include "vxRenderAspect/Graphics/VoxelRenderer.h"
#include <vxGL/Buffer.h>
#include "vxRenderAspect/GpuStructs.h"
#include <vxGL/gl.h>
#include "vxRenderAspect/gl/ObjectManager.h"
#include <vxGL/StateManager.h>
#include "vxRenderAspect/gl/BufferBindingManager.h"
#include <vxRenderAspect/Graphics/Segment.h>
#include "vxRenderAspect/Graphics/CommandList.h"
#include <vxEngineLib/EngineConfig.h>
#include <vxGL/ShaderManager.h>
#include <vxLib/File/FileHandle.h>
#include <vxGL/ProgramPipeline.h>
#include <vxRenderAspect/Graphics/Commands.h>
#include <vxGL/Texture.h>
#include <vxGL/Framebuffer.h>
#include <vxGL/VertexArray.h>

namespace Graphics
{
	struct VoxelRenderer::ColdData
	{
		vx::gl::Texture m_voxelEmmitanceTextures[6];
		vx::gl::Texture m_voxelOpacityTextures[6];
		vx::gl::Texture m_voxelFbTexture;
	};

	VoxelRenderer::VoxelRenderer()
		:m_voxelTextureSize(0),
		m_voxelGridDim(0)
	{

	}

	VoxelRenderer::~VoxelRenderer()
	{

	}

	void VoxelRenderer::initialize(vx::StackAllocator* scratchAllocator, const void*)
	{
		if (FLEXT_NV_conservative_raster != 0)
		{
			s_shaderManager->setDefine("_CONSERVATIVE_RASTER");
		}

		if (FLEXT_NV_shader_atomic_fp16_vector != 0)
		{

		}

		s_shaderManager->loadPipeline(vx::FileHandle("voxelize.pipe"), "voxelize.pipe", scratchAllocator);
		s_shaderManager->loadPipeline(vx::FileHandle("voxelizeLight.pipe"), "voxelizeLight.pipe", scratchAllocator);

		m_voxelTextureSize = s_settings->m_rendererSettings.m_voxelSettings.m_voxelTextureSize;
		m_voxelGridDim = s_settings->m_rendererSettings.m_voxelSettings.m_voxelGridDim;

		m_coldData = std::make_unique<ColdData>();

		createVoxelTextures();
		createVoxelBuffer();
		createVoxelTextureBuffer();
		createFrameBuffer();
	}

	void VoxelRenderer::shutdown()
	{
		m_coldData.reset(nullptr);
	}

	Segment VoxelRenderer::createSegmentVoxelize()
	{
		auto voxelFB = s_objectManager->getFramebuffer("voxelFB");
		auto pipeline = s_shaderManager->getPipeline("voxelize.pipe");

		auto meshCmdBuffer = s_objectManager->getBuffer("meshCmdBuffer");
		auto meshVao = s_objectManager->getVertexArray("meshVao");
		auto meshParamBuffer = s_objectManager->getBuffer("meshParamBuffer");

		/*

		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);


		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);*/

		StateDescription stateDesc = { voxelFB->getId(), meshVao->getId(), pipeline->getId(), meshCmdBuffer->getId(), meshParamBuffer->getId(), false, false, false, false};
		State state;
		state.set(stateDesc);

		//BarrierCommand barrierCmd;
		//barrierCmd.set();

		ViewportCommand viewportCmd;
		viewportCmd.set(vx::uint2(0), vx::uint2(s_settings->m_rendererSettings.m_voxelSettings.m_voxelTextureSize));

		ClearColorCommand clearColorCmd;
		clearColorCmd.set(vx::float4(0));

		ClearCommand clearCmd;
		clearCmd.set(GL_COLOR_BUFFER_BIT);

		MultiDrawElementsIndirectCountCommand drawCmd;
		drawCmd.set(GL_TRIANGLES, GL_UNSIGNED_INT, s_settings->m_rendererSettings.m_maxMeshInstances);

		Segment segment;
		segment.setState(state);

		if (FLEXT_NV_conservative_raster != 0)
		{
			ConservativeRasterCommand rasterCmd;
			rasterCmd.set(1);

			PolygonModeCommand modeCmd;
			modeCmd.set(GL_FRONT_AND_BACK, GL_FILL_RECTANGLE_NV);

			segment.pushCommand(rasterCmd);
			segment.pushCommand(modeCmd);
		}

		segment.pushCommand(viewportCmd);
		segment.pushCommand(clearColorCmd);
		segment.pushCommand(clearCmd);
		segment.pushCommand(drawCmd);

		return segment;
	}

	Segment VoxelRenderer::createSegmentConeTrace()
	{
		/*
		auto emptyVao = m_objectManager.getVertexArray("emptyVao");
		//auto shaderStoragePixelListCmdBuffer = m_objectManager.getBuffer("ShaderStoragePixelListCmdBuffer");

		glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

		vx::gl::StateManager::bindFrameBuffer(m_coneTraceFB);
		glClear(GL_COLOR_BUFFER_BIT);

		vx::gl::StateManager::setViewport(0, 0, m_resolution.x, m_resolution.y);

		auto pPipeline = m_shaderManager.getPipeline("coneTrace.pipe");

		//vx::gl::StateManager::bindBuffer(vx::gl::BufferType::Draw_Indirect_Buffer,shaderStoragePixelListCmdBuffer->getId());
		vx::gl::StateManager::bindPipeline(pPipeline->getId());
		vx::gl::StateManager::bindVertexArray(emptyVao->getId());

		//glDrawArraysIndirect(GL_POINTS, 0);
		glDrawArraysInstanced(GL_POINTS, 0, m_resolution.x / 2, m_resolution.y / 2);
		*/

		Segment segment;

		return segment;
	}

	void VoxelRenderer::getCommandList(CommandList* cmdList)
	{
		auto segmentVoxelize = createSegmentVoxelize();
		//auto segmentConeTrace = createSegmentConeTrace();

		auto lightCmdBuffer = s_objectManager->getBuffer("cmdLight");
		auto voxelFB = s_objectManager->getFramebuffer("voxelFB");
		auto emptyVao = s_objectManager->getVertexArray("emptyVao");
		auto voxelizeLightPipeline = s_shaderManager->getPipeline("voxelizeLight.pipe");
		StateDescription stateDesc = { voxelFB->getId(), emptyVao->getId(), voxelizeLightPipeline->getId(), lightCmdBuffer->getId(), 0, false, false, false, false };
		State state;
		state.set(stateDesc);

		Segment voxelizeLightsSegment;
		voxelizeLightsSegment.setState(state);

		DrawArraysIndirectCommand drawCmd;
		drawCmd.set(GL_POINTS);

		voxelizeLightsSegment.pushCommand(drawCmd);

		if (FLEXT_NV_conservative_raster != 0)
		{
			ConservativeRasterCommand rasterCmd;
			rasterCmd.set(0);

			PolygonModeCommand modeCmd;
			modeCmd.set(GL_FRONT_AND_BACK, GL_FILL);

			voxelizeLightsSegment.pushCommand(rasterCmd);
			voxelizeLightsSegment.pushCommand(modeCmd);
		}

		cmdList->pushSegment(segmentVoxelize, "voxelize");
		cmdList->pushSegment(voxelizeLightsSegment, "voxelizeLights");
		//cmdList->pushSegment(segmentConeTrace, "coneTrace");
	}

	void VoxelRenderer::clearData()
	{
		for (int j = 0; j < 6; ++j)
		{
			glClearTexImage(m_voxelEmmitanceTexturesIDs[j], 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

			glClearTexImage(m_voxelOpacityTextureIDs[j], 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		}
	}

	void VoxelRenderer::bindBuffers()
	{
		auto pVoxelTextureBuffer = s_objectManager->getBuffer("VoxelTextureBuffer");
		auto pVoxelBuffer = s_objectManager->getBuffer("VoxelBuffer");

		gl::BufferBindingManager::bindBaseUniform(6, pVoxelBuffer->getId());
		gl::BufferBindingManager::bindBaseUniform(7, pVoxelTextureBuffer->getId());
	}

	void VoxelRenderer::createVoxelBuffer()
	{
		const u32 textureSizeLod = m_voxelTextureSize;
		const f32 gridsizeLod = m_voxelGridDim;

		Gpu::VoxelBlock voxelBlock;
			auto halfDim = textureSizeLod / 2;
			auto gridHalfSize = gridsizeLod / 2.0f;

			auto gridCellSize = gridHalfSize / halfDim;
			auto invGridCellSize = 1.0f / gridCellSize;

			auto projectionMatrix = vx::MatrixOrthographicRHDX(gridsizeLod, gridsizeLod, 0.0f, gridsizeLod);

			auto backFront = projectionMatrix * vx::MatrixTranslation(0, 0, -gridHalfSize);
			auto leftRight = projectionMatrix * vx::MatrixRotationAxis(vx::g_VXIdentityR1, vx::degToRad(90)) *vx::MatrixTranslation(-gridHalfSize, 0, 0);
			auto topDown = projectionMatrix * vx::MatrixRotationAxis(vx::g_VXIdentityR0, vx::degToRad(90)) *vx::MatrixTranslation(0, -gridHalfSize, 0);
			//auto projectionMatrix = vx::MatrixOrthographicOffCenterRH(-gridHalfSize, gridHalfSize, -gridHalfSize, gridHalfSize, 0.0f, -gridsizeLod);
			voxelBlock.data.projectionMatrix[0] = leftRight;//projectionMatrix * vx::MatrixTranslation(0, 0, -gridHalfSize);
			voxelBlock.data.projectionMatrix[1] = topDown;
			voxelBlock.data.projectionMatrix[2] = backFront;
			voxelBlock.data.dim = textureSizeLod;
			voxelBlock.data.halfDim = halfDim;
			voxelBlock.data.gridCellSize = gridCellSize;
			voxelBlock.data.invGridCellSize = invGridCellSize;

		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
		desc.size = sizeof(Gpu::VoxelBlock);
		desc.flags = 0;
		desc.immutable = 1;
		desc.pData = &voxelBlock;

		s_objectManager->createBuffer("VoxelBuffer", desc);
	}

	void VoxelRenderer::createVoxelTextureBuffer()
	{
		struct VoxelHandles
		{
			u64 u_voxelEmmitanceImage[6];
			u64 u_voxelEmmitanceTexture[6];
			u64 u_voxelOpacityImage[6];
			u64 u_voxelOpacityTexture[6];
		};

		VoxelHandles voxelHandles;

		for (int i = 0; i < 6; ++i)
		{
			voxelHandles.u_voxelEmmitanceImage[i] = m_coldData->m_voxelEmmitanceTextures[i].getImageHandle(0, 1, 0);
			voxelHandles.u_voxelEmmitanceTexture[i] = m_coldData->m_voxelEmmitanceTextures[i].getTextureHandle();

			voxelHandles.u_voxelOpacityImage[i] = m_coldData->m_voxelOpacityTextures[i].getImageHandle(0, 1, 0);
			voxelHandles.u_voxelOpacityTexture[i] = m_coldData->m_voxelOpacityTextures[i].getTextureHandle();
		}

		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
		desc.size = sizeof(VoxelHandles);
		desc.flags = 0;
		desc.immutable = 1;
		desc.pData = &voxelHandles;

		s_objectManager->createBuffer("VoxelTextureBuffer", desc);
	}

	void VoxelRenderer::createVoxelTextures()
	{
		vx::gl::TextureDescription desc;
		if (FLEXT_NV_conservative_raster != 0)
		{
			desc.format = vx::gl::TextureFormat::RGBA16F;
		}
		else
		{
			desc.format = vx::gl::TextureFormat::RGBA8;
		}
		desc.type = vx::gl::TextureType::Texture_3D;
		desc.size = vx::ushort3(m_voxelTextureSize, m_voxelTextureSize, m_voxelTextureSize);
		desc.miplevels = 1;// std::max((u8)1, m_mipcount);

		for (u32 i = 0; i < 6; ++i)
		{
			m_coldData->m_voxelEmmitanceTextures[i].create(desc);
			m_coldData->m_voxelEmmitanceTextures[i].setWrapMode3D(vx::gl::TextureWrapMode::CLAMP_TO_BORDER, vx::gl::TextureWrapMode::CLAMP_TO_BORDER, vx::gl::TextureWrapMode::CLAMP_TO_BORDER);
			m_coldData->m_voxelEmmitanceTextures[i].setFilter(vx::gl::TextureFilter::LINEAR, vx::gl::TextureFilter::LINEAR);

			m_coldData->m_voxelEmmitanceTextures[i].makeTextureResident();
			m_coldData->m_voxelEmmitanceTextures[i].makeImageResident(0, 1, 0, vx::gl::TextureAccess::Write);

			m_voxelEmmitanceTexturesIDs[i] = m_coldData->m_voxelEmmitanceTextures[i].getId();

			m_coldData->m_voxelOpacityTextures[i].create(desc);
			m_coldData->m_voxelOpacityTextures[i].setFilter(vx::gl::TextureFilter::LINEAR, vx::gl::TextureFilter::LINEAR);
			m_coldData->m_voxelOpacityTextures[i].makeTextureResident();
			m_coldData->m_voxelOpacityTextures[i].makeImageResident(0, 1, 0, vx::gl::TextureAccess::Write);
			m_voxelOpacityTextureIDs[i] = m_coldData->m_voxelOpacityTextures[i].getId();
		}
	}

	void VoxelRenderer::createFrameBuffer()
	{
		vx::gl::TextureDescription desc;
		desc.type = vx::gl::TextureType::Texture_2D;
		desc.format = vx::gl::TextureFormat::R8;
		desc.size = vx::ushort3(m_voxelTextureSize, m_voxelTextureSize, 1);
		desc.samples = 8;
		m_coldData->m_voxelFbTexture.create(desc);

		auto voxelFBSid = s_objectManager->createFramebuffer("voxelFB");
		auto voxelFB = s_objectManager->getFramebuffer(voxelFBSid);

		voxelFB->attachTexture(vx::gl::Attachment::Color0, m_coldData->m_voxelFbTexture, 0);
		voxelFB->drawBuffer(vx::gl::Attachment::Color0);
		//glNamedFramebufferDrawBuffer(voxelFB->getId(), GL_COLOR_ATTACHMENT0);
	}

	void VoxelRenderer::debug(const vx::gl::VertexArray &vao, const vx::uint2 &resolution)
	{
		/*glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		vx::gl::StateManager::enable(vx::gl::Capabilities::Blend);
		vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);

		vx::gl::StateManager::setViewport(0, 0, resolution.x, resolution.y);
		vx::gl::StateManager::bindFrameBuffer(0);

		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glLineWidth(1.0f);

		vx::gl::StateManager::bindPipeline(m_pipelineDebug);
		vx::gl::StateManager::bindVertexArray(vao);

		glDrawArraysInstanced(GL_POINTS, 0, m_voxelTextureSize, m_voxelTextureSize * m_voxelTextureSize);

		vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);
		vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);*/
	}
}