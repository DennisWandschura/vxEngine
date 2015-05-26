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

namespace gl
{
	class ObjectManager;
}

#include <vxLib/gl/Framebuffer.h>
#include <vxLib/gl/Texture.h>
#include <vxLib/gl/Buffer.h>
#include <vxLib/memory.h>

class VoxelRenderer
{
	struct ColdData
	{
		vx::gl::Texture m_voxelFbTexture;
		vx::gl::Texture m_voxelEmmitanceTextures[6];
		vx::gl::Texture m_voxelOpacityTexture;
	};

	u32 m_pipelineVoxelize{0};
	u32 m_pipelineGs{ 0 };
	u32 m_pipelineDebug{0};
	u32 m_pipelineMipmap{ 0 };
	vx::gl::Framebuffer m_voxelFB;
	u16 m_voxelTextureSize{0};
	u8 m_mipcount{ 1 };
	u32 m_voxelEmmitanceTexturesId[6];
	u32 m_voxelOpacityTextureId;

	std::unique_ptr<ColdData> m_pColdData;

	void createVoxelBuffer(gl::ObjectManager* objectManager);
	void createVoxelTextureBuffer(gl::ObjectManager* objectManager);
	void createVoxelTextures();
	void createFrameBuffer();

public:
	VoxelRenderer() = default;

	void initialize(u16 voxelTextureSize, const vx::gl::ShaderManager &shaderManager, gl::ObjectManager* objectManager);

	void bindBuffers(const gl::ObjectManager &objectManager);

	void clearTextures();
	void voxelizeScene(u32 count, const vx::gl::Buffer &indirectCmdBuffer, const vx::gl::VertexArray &vao);
	void createMipmaps();

	void debug(const vx::gl::VertexArray &vao, vx::uint2 &resolution);
};
