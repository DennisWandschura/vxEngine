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
#include "VoxelRenderer.h"
#include <vxLib/gl/StateManager.h>
#include <vxLib/gl/Buffer.h>
#include <vxLib/gl/ProgramPipeline.h>
#include <vxLib/gl/ShaderManager.h>
#include <vxLib/gl/gl.h>
#include "GpuStructs.h"
#include "gl/BufferBindingManager.h"
#include "gl/ObjectManager.h"

const u32 voxelLodCount = 4;

struct VoxelRenderer::ColdData
{
	vx::gl::Texture m_voxelFbTexture;
	vx::gl::Texture m_voxelEmmitanceTextures[6];
	vx::gl::Texture m_voxelOpacityTextures[6];
};

VoxelRenderer::VoxelRenderer()
{

}

VoxelRenderer::~VoxelRenderer()
{

}

void VoxelRenderer::initialize(u16 voxelTextureSize, const vx::gl::ShaderManager &shaderManager, gl::ObjectManager* objectManager)
{
	//m_voxelTextureSize = voxelTextureSize;
	m_voxelTextureSize = 128;

	//m_mipcount = std::log2(m_voxelTextureSize);
	m_mipcount = voxelLodCount;

	m_pColdData = vx::make_unique<ColdData>();
	auto voxel_debug = shaderManager.getPipeline("voxel_debug.pipe");
	auto voxelMipmap = shaderManager.getPipeline("voxelMipmap.pipe");

	VX_ASSERT(voxel_debug);
	VX_ASSERT(voxelMipmap);

	m_pipelineDebug = voxel_debug->getId();
	m_pipelineMipmap = voxelMipmap->getId();

	createVoxelTextures();
	createFrameBuffer(objectManager);

	createVoxelBuffer(objectManager);
	createVoxelTextureBuffer(objectManager);
}

void VoxelRenderer::createVoxelBuffer(gl::ObjectManager* objectManager)
{
	const __m128 axisX = { 1, 0, 0, 0 };
	const __m128 axisY = { 0, 1, 0, 0 };

	const u32 sizeLod = m_voxelTextureSize;
	const f32 gridsizeLod = 20;

	VoxelBlock voxelBlock;
	for (u32 i = 0; i < voxelLodCount; ++i)
	{
		auto halfDim = sizeLod / 2;
		auto gridHalfSize = gridsizeLod / 2.0f;

		auto gridCellSize = gridHalfSize / halfDim;
		auto invGridCellSize = 1.0f / gridCellSize;

		auto projectionMatrix = vx::MatrixOrthographicOffCenterRH(-gridHalfSize, gridHalfSize, -gridHalfSize, gridHalfSize, 0.0f, -gridsizeLod);
		voxelBlock.data[i].projectionMatrix = projectionMatrix * vx::MatrixTranslation(0, 0, gridHalfSize);
		voxelBlock.data[i].dim = sizeLod;
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

	objectManager->createBuffer("VoxelBuffer", desc);
}

void VoxelRenderer::createVoxelTextureBuffer(gl::ObjectManager* objectManager)
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
		voxelHandles.u_voxelEmmitanceImage[i] = m_pColdData->m_voxelEmmitanceTextures[i].getImageHandle(0, 1, 0);
		voxelHandles.u_voxelEmmitanceTexture[i] = m_pColdData->m_voxelEmmitanceTextures[i].getTextureHandle();

		voxelHandles.u_voxelOpacityImage[i] = m_pColdData->m_voxelOpacityTextures[i].getImageHandle(0, 1, 0);
		voxelHandles.u_voxelOpacityTexture[i] = m_pColdData->m_voxelOpacityTextures[i].getTextureHandle();
	}

	vx::gl::BufferDescription desc;
	desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
	desc.size = sizeof(VoxelHandles);
	desc.flags = 0;
	desc.immutable = 1;
	desc.pData = &voxelHandles;

	objectManager->createBuffer("VoxelTextureBuffer", desc);
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
		m_pColdData->m_voxelEmmitanceTextures[i].create(desc);
		m_pColdData->m_voxelEmmitanceTextures[i].setWrapMode3D(vx::gl::TextureWrapMode::CLAMP_TO_EDGE, vx::gl::TextureWrapMode::CLAMP_TO_EDGE, vx::gl::TextureWrapMode::CLAMP_TO_EDGE);
		m_pColdData->m_voxelEmmitanceTextures[i].setFilter(vx::gl::TextureFilter::LINEAR, vx::gl::TextureFilter::LINEAR);

		m_pColdData->m_voxelEmmitanceTextures[i].makeTextureResident();
		glMakeImageHandleResidentARB(m_pColdData->m_voxelEmmitanceTextures[i].getImageHandle(0, 1, 0), GL_WRITE_ONLY);

		m_voxelEmmitanceTexturesId[i] = m_pColdData->m_voxelEmmitanceTextures[i].getId();

		m_pColdData->m_voxelOpacityTextures[i].create(desc);
		m_pColdData->m_voxelOpacityTextures[i].setFilter(vx::gl::TextureFilter::LINEAR, vx::gl::TextureFilter::LINEAR);
		m_pColdData->m_voxelOpacityTextures[i].makeTextureResident();
		glMakeImageHandleResidentARB(m_pColdData->m_voxelOpacityTextures[i].getImageHandle(0, 1, 0), GL_READ_WRITE);
		m_voxelOpacityTextureId[i] = m_pColdData->m_voxelOpacityTextures[i].getId();
	}
}

void VoxelRenderer::createFrameBuffer(gl::ObjectManager* objectManager)
{
	vx::gl::TextureDescription desc;
	desc.type = vx::gl::TextureType::Texture_2D_MS;
	desc.format = vx::gl::TextureFormat::R8;
	desc.size = vx::ushort3(m_voxelTextureSize, m_voxelTextureSize, 1);
	desc.samples = 8;
	m_pColdData->m_voxelFbTexture.create(desc);

	auto voxelFBSid = objectManager->createFramebuffer("voxelFB");
	auto voxelFB = objectManager->getFramebuffer(voxelFBSid);

	voxelFB->attachTexture(vx::gl::Attachment::Color0, m_pColdData->m_voxelFbTexture, 0);
	glNamedFramebufferDrawBuffer(voxelFB->getId(), GL_COLOR_ATTACHMENT0);
}

void VoxelRenderer::clearTextures()
{
	//for (u32 miplevel = 0; miplevel < m_mipcount; ++miplevel)
	//{
		for (int j = 0; j < 6; ++j)
		{
			glClearTexImage(m_voxelEmmitanceTexturesId[j], 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

			glClearTexImage(m_voxelOpacityTextureId[j], 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		}
	//}
}

void VoxelRenderer::debug(const vx::gl::VertexArray &vao, vx::uint2 &resolution)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);
}