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
	class StackAllocator;
}

#include <vxLib/types.h>
#include <vxEngineLib/MessageListener.h>
#include <vxEngineLib/Graphics/CommandQueue.h>
#include <vxEngineLib/Graphics/RenderUpdateTask.h>

namespace Graphics
{
	class RenderLayer : public ::vx::MessageListener
	{
	public:
		virtual ~RenderLayer() {}

		virtual void createRenderPasses() = 0;

		virtual void getRequiredMemory(u64* heapSizeBuffer, u32* bufferCount, u64* heapSizeTexture, u32* textureCount, u64* heapSizeRtDs, u32* rtDsCount) = 0;

		virtual bool initialize(vx::StackAllocator* allocator) = 0;
		virtual void shudown() = 0;

		virtual void update() = 0;

		virtual void queueUpdate(const RenderUpdateTaskType type, const u8* data, u32 dataSize) = 0;

		virtual void buildCommandLists() = 0;
		virtual void submitCommandLists(Graphics::CommandQueue* queue) = 0;

		virtual u32 getCommandListCount() const = 0;
	};
}