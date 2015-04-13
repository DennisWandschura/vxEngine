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
	static const U32 s_voxelDim = 128u;

	struct ColdData
	{
		vx::gl::Texture m_voxelFbTexture;
	};

	const vx::gl::ProgramPipeline* m_pipelineVoxelize{nullptr};
	const vx::gl::ProgramPipeline* m_pipelineDebug{nullptr};
	vx::gl::Framebuffer m_voxelFB;
	vx::gl::Texture m_voxelOpacityTexture;
	vx::gl::Texture m_voxelOpacityTextureLod1;
	vx::gl::Texture m_voxelEmmitanceTextures[6];
	vx::gl::Texture m_voxelEmmitanceTexturesLod1[6];
	std::unique_ptr<ColdData> m_pColdData;

	void createVoxelBuffer(BufferManager* bufferManager);
	void createVoxelTextureBuffer(BufferManager* bufferManager);
	void createVoxelTextures();
	void createFrameBuffer();

public:
	VoxelRenderer() = default;

	void initialize(const vx::gl::ShaderManager &shaderManager, BufferManager* bufferManager);

	void bindBuffers(const BufferManager &bufferManager);

	void clearTextures();
	void voxelizeScene(U32 count, const vx::gl::Buffer &indirectCmdBuffer, const vx::gl::VertexArray &vao);

	void debug(const vx::gl::VertexArray &vao);
};
