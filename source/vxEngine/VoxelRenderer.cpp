#include "VoxelRenderer.h"
#include <vxLib/gl/StateManager.h>
#include <vxLib/gl/Buffer.h>
#include <vxLib/gl/ProgramPipeline.h>
#include <vxLib/gl/ShaderManager.h>
#include <vxLib/gl/gl.h>
#include "BufferBlocks.h"
#include "BufferBindingManager.h"
#include "BufferManager.h"

void VoxelRenderer::initialize(const vx::gl::ShaderManager &shaderManager, BufferManager* bufferManager)
{
	m_pColdData = std::make_unique<ColdData>();
	m_pipelineVoxelize = shaderManager.getPipeline("voxelize.pipe");
	m_pipelineDebug = shaderManager.getPipeline("voxel_debug.pipe");

	createVoxelTextures();
	createFrameBuffer();

	createVoxelBuffer(bufferManager);
	createVoxelTextureBuffer(bufferManager);
}

void VoxelRenderer::createVoxelBuffer(BufferManager* bufferManager)
{
	const __m128 axisX = { 1, 0, 0, 0 };
	const __m128 axisY = { 0, 1, 0, 0 };

	const U32 halfDim = s_voxelDim / 2;

	F32 gridSize[2];
	gridSize[0] = 20;
	gridSize[1] = 32;

	F32 gridCellSize[2];
	F32 invGridCellSize[2];

	vx::mat4 voxelPvMatrix[2];
	for (int i = 0; i < 2; ++i)
	{
		F32 gridHalfSize = gridSize[i] / 2.0f;
		gridCellSize[i] = gridHalfSize / halfDim;
		invGridCellSize[i] = 1.0f / gridCellSize[i];

		auto projectionMatrix = vx::MatrixOrthographicOffCenterRH(-gridHalfSize, gridHalfSize, -gridHalfSize, gridHalfSize, 0.0f, -gridSize[i]);
		voxelPvMatrix[i] = projectionMatrix * vx::MatrixTranslation(0, 0, gridHalfSize);
	}

	VoxelBlock voxelBlock;
	voxelBlock.projectionMatrix[0] = voxelPvMatrix[0];
	voxelBlock.projectionMatrix[1] = voxelPvMatrix[1];
	voxelBlock.dim = s_voxelDim;
	voxelBlock.halfDim = halfDim;
	voxelBlock.gridCellSize[0] = gridCellSize[0];
	voxelBlock.gridCellSize[1] = gridCellSize[1];
	voxelBlock.invGridCellSize[0] = invGridCellSize[0];
	voxelBlock.invGridCellSize[1] = invGridCellSize[1];

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
		U64 u_voxelEmmitanceImage[2][6];
		U64 u_voxelEmmitanceTexture[2][6];
		U64 u_voxelOpacityImage[2];
		U64 u_voxelOpacityTexture[2];
	};

	VoxelHandles voxelHandles;

	for (int i = 0; i < 6; ++i)
	{
		voxelHandles.u_voxelEmmitanceImage[0][i] = m_voxelEmmitanceTextures[i].getImageHandle(0, 1, 0);
		voxelHandles.u_voxelEmmitanceImage[1][i] = m_voxelEmmitanceTexturesLod1[i].getImageHandle(0, 1, 0);

		voxelHandles.u_voxelEmmitanceTexture[0][i] = m_voxelEmmitanceTextures[i].getTextureHandle();
		voxelHandles.u_voxelEmmitanceTexture[1][i] = m_voxelEmmitanceTexturesLod1[i].getTextureHandle();
	}

	voxelHandles.u_voxelOpacityImage[0] = m_voxelOpacityTexture.getImageHandle(0, 1, 0);
	voxelHandles.u_voxelOpacityImage[1] = m_voxelOpacityTextureLod1.getImageHandle(0, 1, 0);

	voxelHandles.u_voxelOpacityTexture[0] = m_voxelOpacityTexture.getTextureHandle();
	voxelHandles.u_voxelOpacityTexture[1] = m_voxelOpacityTextureLod1.getTextureHandle();

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
	desc.type = vx::gl::TextureType::Texture_3D;
	desc.format = vx::gl::TextureFormat::R32UI;
	desc.size = vx::ushort3(s_voxelDim, s_voxelDim, s_voxelDim);
	desc.miplevels = 1;

	for (U32 i = 0; i < 6; ++i)
	{
		m_voxelEmmitanceTextures[i].create(desc);
		m_voxelEmmitanceTextures[i].makeTextureResident();
		glMakeImageHandleResidentARB(m_voxelEmmitanceTextures[i].getImageHandle(0, 1, 0), GL_READ_WRITE);

		m_voxelEmmitanceTexturesLod1[i].create(desc);
		m_voxelEmmitanceTexturesLod1[i].makeTextureResident();
		glMakeImageHandleResidentARB(m_voxelEmmitanceTexturesLod1[i].getImageHandle(0, 1, 0), GL_READ_WRITE);
	}

	m_voxelOpacityTexture.create(desc);
	m_voxelOpacityTexture.makeTextureResident();
	glMakeImageHandleResidentARB(m_voxelOpacityTexture.getImageHandle(0, 1, 0), GL_READ_WRITE);

	m_voxelOpacityTextureLod1.create(desc);
	m_voxelOpacityTextureLod1.makeTextureResident();
	glMakeImageHandleResidentARB(m_voxelOpacityTextureLod1.getImageHandle(0, 1, 0), GL_READ_WRITE);
}

void VoxelRenderer::createFrameBuffer()
{
	vx::gl::TextureDescription desc;
	desc.type = vx::gl::TextureType::Texture_2D_MS;
	desc.format = vx::gl::TextureFormat::R8;
	desc.size = vx::ushort3(s_voxelDim, s_voxelDim, 1);
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
	for (int i = 0; i < 6; ++i)
	{
		m_voxelEmmitanceTextures[i].clearImage(0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
		m_voxelEmmitanceTexturesLod1[i].clearImage(0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
	}

	m_voxelOpacityTexture.clearImage(0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
	m_voxelOpacityTextureLod1.clearImage(0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
}

void VoxelRenderer::voxelizeScene(U32 count, const vx::gl::Buffer &indirectCmdBuffer, const vx::gl::VertexArray &vao)
{
	vx::gl::StateManager::setClearColor(0, 0, 0, 0);
	vx::gl::StateManager::setViewport(0, 0, s_voxelDim, s_voxelDim);
	vx::gl::StateManager::bindFrameBuffer(m_voxelFB);

	glClear(GL_COLOR_BUFFER_BIT);

	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Cull_Face);

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);

	auto gs = m_pipelineVoxelize->getGeometryShader();
	auto fs = m_pipelineVoxelize->getFragmentShader();

	vx::gl::StateManager::bindPipeline(m_pipelineVoxelize->getId());
	vx::gl::StateManager::bindVertexArray(vao);

	indirectCmdBuffer.bind();
	for (int i = 0; i < 2; ++i)
	{
		glProgramUniform1i(gs, 0, i);
		glProgramUniform1i(fs, 0, i);

		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, count, sizeof(vx::gl::DrawElementsIndirectCommand));
	}

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	vx::gl::StateManager::enable(vx::gl::Capabilities::Cull_Face);
}

void VoxelRenderer::debug(const vx::gl::VertexArray &vao)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	vx::gl::StateManager::enable(vx::gl::Capabilities::Blend);
	vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);

	vx::gl::StateManager::setViewport(0, 0, 1920, 1080);
	vx::gl::StateManager::bindFrameBuffer(0);

	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glLineWidth(1.0f);

	vx::gl::StateManager::bindPipeline(m_pipelineDebug->getId());
	vx::gl::StateManager::bindVertexArray(vao);

	glDrawArraysInstanced(GL_POINTS, 0, s_voxelDim, s_voxelDim * s_voxelDim);

	vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);
}