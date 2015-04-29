#pragma once

namespace vx
{
	namespace gl
	{
		class ProgramPipeline;
		class Buffer;
		class VertexArray;
		class ShaderManager;
	}
}

class BufferManager;

#include <vxLib/gl/Framebuffer.h>
#include <vxLib/gl/Texture.h>
#include <vxLib/gl/Buffer.h>
#include <memory>

class VoxelRenderer
{
	struct ColdData
	{
		vx::gl::Texture m_voxelFbTexture;
		vx::gl::Texture m_voxelEmmitanceTextures[6];
		vx::gl::Texture m_voxelOpacityTexture;
	};

	U32 m_pipelineVoxelize{0};
	U32 m_pipelineDebug{0};
	U32 m_pipelineMipmap{ 0 };
	vx::gl::Framebuffer m_voxelFB;
	U16 m_voxelTextureSize{0};
	U8 m_mipcount{ 1 };
	U32 m_voxelEmmitanceTexturesId[6];
	U32 m_voxelOpacityTextureId;

	std::unique_ptr<ColdData> m_pColdData;

	void createVoxelBuffer( BufferManager* bufferManager);
	void createVoxelTextureBuffer(BufferManager* bufferManager);
	void createVoxelTextures();
	void createFrameBuffer();

public:
	VoxelRenderer() = default;

	void initialize(U16 voxelTextureSize, const vx::gl::ShaderManager &shaderManager, BufferManager* bufferManager);

	void bindBuffers(const BufferManager &bufferManager);

	void clearTextures();
	void voxelizeScene(U32 count, const vx::gl::Buffer &indirectCmdBuffer, const vx::gl::VertexArray &vao);
	void createMipmaps();

	void debug(const vx::gl::VertexArray &vao, vx::uint2 &resolution);
};
