#pragma once

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

namespace vx
{
	namespace gl
	{
		class VertexArray;
	}
}

#include "Renderer.h"
#include <memory>
#include <vxLib/math/Vector.h>

namespace Graphics
{
	class Segment;

	class VoxelRenderer : public Renderer
	{
		struct ColdData;

		u32 m_voxelEmmitanceTexturesIDs[6];
		u32 m_voxelOpacityTextureIDs[6];
		u32 m_voxelTextureSize;
		u32 m_voxelGridDim;
		std::unique_ptr<ColdData> m_coldData;

		void createVoxelTextures();
		void createVoxelTextureBuffer();
		void createVoxelBuffer();
		void createFrameBuffer();

		Segment createSegmentVoxelize();
		Segment createSegmentConeTrace();

	public:
		VoxelRenderer();
		~VoxelRenderer();

		void initialize(vx::StackAllocator* scratchAllocator) override;
		void shutdown() override;

		void update() override;

		void getCommandList(CommandList* cmdList) override;

		void clearData() override;
		void bindBuffers() override;

		void debug(const vx::gl::VertexArray &vao, const vx::uint2 &resolution);
	};
}