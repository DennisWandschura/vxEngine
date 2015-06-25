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

	void VoxelRenderer::initialize(vx::StackAllocator* scratchAllocator)
	{
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

	void VoxelRenderer::update()
	{

	}

	Segment VoxelRenderer::createSegmentVoxelize()
	{
		/*
		auto voxelFB = m_objectManager.getFramebuffer("voxelFB");
		auto emptyVao = m_objectManager.getVertexArray("emptyVao");
		auto pipeline = m_shaderManager.getPipeline("voxelize.pipe");
		auto voxelizeLightPipeline = m_shaderManager.getPipeline("voxelizeLight.pipe");

		auto voxelSize = 128;

		auto lightCount = m_sceneRenderer.getLightCount();

		vx::gl::StateManager::setClearColor(0, 0, 0, 0);
		vx::gl::StateManager::bindFrameBuffer(*voxelFB);

		glClear(GL_COLOR_BUFFER_BIT);

		vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);
		vx::gl::StateManager::disable(vx::gl::Capabilities::Cull_Face);

		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);

		vx::gl::StateManager::bindPipeline(*pipeline);
		vx::gl::StateManager::bindVertexArray(vao);
		vx::gl::StateManager::bindBuffer(vx::gl::BufferType::Draw_Indirect_Buffer, cmdBuffer);
		vx::gl::StateManager::bindBuffer(vx::gl::BufferType::Parameter_Buffer, paramBuffer);

		vx::gl::StateManager::setViewport(0, 0, voxelSize, voxelSize);
		glMultiDrawElementsIndirectCountARB(GL_TRIANGLES, GL_UNSIGNED_INT, 0, 0, 150, sizeof(vx::gl::DrawElementsIndirectCommand));

		vx::gl::StateManager::bindVertexArray(*emptyVao);
		vx::gl::StateManager::bindPipeline(*voxelizeLightPipeline);
		glDrawArrays(GL_POINTS, 0, lightCount);

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);

		vx::gl::StateManager::enable(vx::gl::Capabilities::Cull_Face);
		*/
		Segment segment;

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
		auto segmentConeTrace = createSegmentConeTrace();

		//cmdList->pushSegment(segmentVoxelize, "voxelize");
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

		VoxelBlock voxelBlock;
		//for (u32 i = 0; i < voxelLodCount; ++i)
		u32 i = 0;
		{
			auto halfDim = textureSizeLod / 2;
			auto gridHalfSize = gridsizeLod / 2.0f;

			auto gridCellSize = gridHalfSize / halfDim;
			auto invGridCellSize = 1.0f / gridCellSize;

			auto projectionMatrix = vx::MatrixOrthographicRHDX(gridsizeLod, gridsizeLod, 0.0f, -gridsizeLod);

			//auto projectionMatrix = vx::MatrixOrthographicOffCenterRH(-gridHalfSize, gridHalfSize, -gridHalfSize, gridHalfSize, 0.0f, -gridsizeLod);
			voxelBlock.data[i].projectionMatrix = projectionMatrix * vx::MatrixTranslation(0, 0, gridHalfSize);
			voxelBlock.data[i].dim = textureSizeLod;
			voxelBlock.data[i].halfDim = halfDim;
			voxelBlock.data[i].gridCellSize = gridCellSize;
			voxelBlock.data[i].invGridCellSize = invGridCellSize;
		}

		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
		desc.size = sizeof(VoxelBlock);
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
		desc.format = vx::gl::TextureFormat::RGBA8;
		desc.type = vx::gl::TextureType::Texture_3D;
		desc.size = vx::ushort3(m_voxelTextureSize, m_voxelTextureSize, m_voxelTextureSize);
		desc.miplevels = 1;// std::max((u8)1, m_mipcount);

		for (u32 i = 0; i < 6; ++i)
		{
			m_coldData->m_voxelEmmitanceTextures[i].create(desc);
			m_coldData->m_voxelEmmitanceTextures[i].setWrapMode3D(vx::gl::TextureWrapMode::CLAMP_TO_EDGE, vx::gl::TextureWrapMode::CLAMP_TO_EDGE, vx::gl::TextureWrapMode::CLAMP_TO_EDGE);
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
		desc.type = vx::gl::TextureType::Texture_2D_MS;
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