#include "VoxelRenderer.h"
#include <vxLib/gl/StateManager.h>
#include <vxLib/gl/Buffer.h>
#include <vxLib/gl/ProgramPipeline.h>
#include <vxLib/gl/ShaderManager.h>
#include <vxLib/gl/gl.h>
#include "GpuStructs.h"
#include "BufferBindingManager.h"
#include "BufferManager.h"

void VoxelRenderer::initialize(U16 voxelTextureSize, const vx::gl::ShaderManager &shaderManager, BufferManager* bufferManager)
{
	m_voxelTextureSize = voxelTextureSize;
	m_mipcount = std::log2(m_voxelTextureSize);

	m_pColdData = std::make_unique<ColdData>();
	m_pipelineVoxelize = shaderManager.getPipeline("voxelize.pipe")->getId();
	m_pipelineDebug = shaderManager.getPipeline("voxel_debug.pipe")->getId();
	m_pipelineMipmap = shaderManager.getPipeline("voxelMipmap.pipe")->getId();

	createVoxelTextures();
	createFrameBuffer();

	createVoxelBuffer(bufferManager);
	createVoxelTextureBuffer(bufferManager);
}

void VoxelRenderer::createVoxelBuffer(BufferManager* bufferManager)
{
	const __m128 axisX = { 1, 0, 0, 0 };
	const __m128 axisY = { 0, 1, 0, 0 };

	const U32 halfDim = m_voxelTextureSize / 2;

	F32 gridSize = 10.0f;

	F32 gridHalfSize = gridSize / 2.0f;
	auto gridCellSize = gridHalfSize / halfDim;
	auto invGridCellSize = 1.0f / gridCellSize;

	auto projectionMatrix = vx::MatrixOrthographicOffCenterRH(-gridHalfSize, gridHalfSize, -gridHalfSize, gridHalfSize, 0.0f, -gridSize);
	auto voxelPvMatrix = projectionMatrix * vx::MatrixTranslation(0, 0, gridHalfSize);

	VoxelBlock voxelBlock;
	voxelBlock.projectionMatrix = voxelPvMatrix;
	voxelBlock.dim = m_voxelTextureSize;
	voxelBlock.halfDim = halfDim;
	voxelBlock.gridCellSize = gridCellSize;
	voxelBlock.invGridCellSize = invGridCellSize;

	vx::gl::BufferDescription desc;
	desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
	desc.size = sizeof(VoxelBlock);
	desc.flags = 0;
	desc.immutable = 1;
	desc.pData = &voxelBlock;

	bufferManager->createBuffer("VoxelBuffer", desc);
}

void VoxelRenderer::createVoxelTextureBuffer(BufferManager* bufferManager)
{
	struct VoxelHandles
	{
		U64 u_voxelEmmitanceImage[6];
		U64 u_voxelEmmitanceTexture[6];
		U64 u_voxelOpacityImage;
		U64 u_voxelOpacityTexture;
	};

	VoxelHandles voxelHandles;

	for (int i = 0; i < 6; ++i)
	{
		voxelHandles.u_voxelEmmitanceImage[i] = m_pColdData->m_voxelEmmitanceTextures[i].getImageHandle(0, 1, 0);
		voxelHandles.u_voxelEmmitanceTexture[i] = m_pColdData->m_voxelEmmitanceTextures[i].getTextureHandle();
	}

	voxelHandles.u_voxelOpacityImage = m_pColdData->m_voxelOpacityTexture.getImageHandle(0, 1, 0);
	voxelHandles.u_voxelOpacityTexture = m_pColdData->m_voxelOpacityTexture.getTextureHandle();

	vx::gl::BufferDescription desc;
	desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
	desc.size = sizeof(VoxelHandles);
	desc.flags = 0;
	desc.immutable = 1;
	desc.pData = &voxelHandles;

	bufferManager->createBuffer("VoxelTextureBuffer", desc);
}

void VoxelRenderer::createVoxelTextures()
{
	vx::gl::TextureDescription desc;
	desc.format = vx::gl::TextureFormat::RGBA8;
	desc.type = vx::gl::TextureType::Texture_3D;
	desc.size = vx::ushort3(m_voxelTextureSize, m_voxelTextureSize, m_voxelTextureSize);
	desc.miplevels = std::max((U8)1, m_mipcount);

	for (U32 i = 0; i < 6; ++i)
	{
		m_pColdData->m_voxelEmmitanceTextures[i].create(desc);
		m_pColdData->m_voxelEmmitanceTextures[i].setFilter(vx::gl::TextureFilter::LINEAR_MIPMAP_LINEAR, vx::gl::TextureFilter::LINEAR);
		m_pColdData->m_voxelEmmitanceTextures[i].makeTextureResident();
		glMakeImageHandleResidentARB(m_pColdData->m_voxelEmmitanceTextures[i].getImageHandle(0, 1, 0), GL_READ_WRITE);

		m_voxelEmmitanceTexturesId[i] = m_pColdData->m_voxelEmmitanceTextures[i].getId();
	}

	m_pColdData->m_voxelOpacityTexture.create(desc);
	m_pColdData->m_voxelOpacityTexture.setFilter(vx::gl::TextureFilter::LINEAR_MIPMAP_LINEAR, vx::gl::TextureFilter::LINEAR);
	m_pColdData->m_voxelOpacityTexture.makeTextureResident();
	glMakeImageHandleResidentARB(m_pColdData->m_voxelOpacityTexture.getImageHandle(0, 1, 0), GL_READ_WRITE);

	m_voxelOpacityTextureId = m_pColdData->m_voxelOpacityTexture.getId();
}

void VoxelRenderer::createFrameBuffer()
{
	vx::gl::TextureDescription desc;
	desc.type = vx::gl::TextureType::Texture_2D_MS;
	desc.format = vx::gl::TextureFormat::R8;
	desc.size = vx::ushort3(m_voxelTextureSize, m_voxelTextureSize, 1);
	desc.samples = 8;
	m_pColdData->m_voxelFbTexture.create(desc);

	m_voxelFB.create();
	m_voxelFB.attachTexture(vx::gl::Attachment::Color0, m_pColdData->m_voxelFbTexture, 0);
	glNamedFramebufferDrawBuffer(m_voxelFB.getId(), GL_COLOR_ATTACHMENT0);
}

void VoxelRenderer::bindBuffers(const BufferManager &bufferManager)
{
	auto pVoxelBuffer = bufferManager.getBuffer("VoxelBuffer");
	auto pVoxelTextureBuffer = bufferManager.getBuffer("VoxelTextureBuffer");

	BufferBindingManager::bindBaseUniform(6, pVoxelBuffer->getId());
	BufferBindingManager::bindBaseUniform(7, pVoxelTextureBuffer->getId());
}

void VoxelRenderer::clearTextures()
{
	for (U32 miplevel = 0; miplevel < m_mipcount; ++miplevel)
	{
		for (int j = 0; j < 6; ++j)
		{
			glClearTexImage(m_voxelEmmitanceTexturesId[j], miplevel, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		}

		glClearTexImage(m_voxelOpacityTextureId, miplevel, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	}
}

void VoxelRenderer::voxelizeScene(U32 count, const vx::gl::Buffer &indirectCmdBuffer, const vx::gl::VertexArray &vao)
{
	vx::gl::StateManager::setClearColor(0, 0, 0, 0);
	vx::gl::StateManager::setViewport(0, 0, m_voxelTextureSize, m_voxelTextureSize);
	vx::gl::StateManager::bindFrameBuffer(m_voxelFB);

	glClear(GL_COLOR_BUFFER_BIT);

	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Cull_Face);

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);

	vx::gl::StateManager::bindPipeline(m_pipelineVoxelize);
	vx::gl::StateManager::bindVertexArray(vao);

	indirectCmdBuffer.bind();
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, count, sizeof(vx::gl::DrawElementsIndirectCommand));

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	vx::gl::StateManager::enable(vx::gl::Capabilities::Cull_Face);
}

void VoxelRenderer::createMipmaps()
{
	int srcLevel = 0;
	int dstLevel = 1;
	auto dstResolution = m_voxelTextureSize / 2;

	for (U8 i = 1; i <= m_mipcount; ++i)
	{
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

		vx::gl::StateManager::bindPipeline(m_pipelineMipmap);

		for (int j = 0; j < 6; ++j)
		{
			glBindImageTexture(0, m_voxelEmmitanceTexturesId[j], srcLevel, 1, 0, GL_READ_ONLY, GL_RGBA8);
			glBindImageTexture(1, m_voxelEmmitanceTexturesId[j], dstLevel, 1, 0, GL_WRITE_ONLY, GL_RGBA8);

			glDrawArraysInstanced(GL_POINTS, 0, dstResolution, dstResolution * dstResolution);
		}

		glBindImageTexture(0, m_voxelOpacityTextureId, srcLevel, 1, 0, GL_READ_ONLY, GL_RGBA8);
		glBindImageTexture(1, m_voxelOpacityTextureId, dstLevel, 1, 0, GL_WRITE_ONLY, GL_RGBA8);

		glDrawArraysInstanced(GL_POINTS, 0, dstResolution, dstResolution * dstResolution);

		++srcLevel;
		++dstLevel;
		dstResolution /= 2;
	}
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