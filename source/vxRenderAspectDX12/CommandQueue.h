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

struct ID3D12CommandQueue;
struct D3D12_COMMAND_QUEUE_DESC;
struct ID3D12CommandList;
struct ID3D12Fence;

#include "d3d.h"
#include <vxEngineLib/Graphics/CommandQueue.h>
#include <memory>

namespace d3d
{
	class CommandQueue : public ::Graphics::CommandQueue
	{
		std::unique_ptr<ID3D12CommandList*[]> m_lists;
		u32 m_listCount;
		u32 m_listCapacity;
		Object<ID3D12CommandQueue> m_commandQueue;
		u64 m_currentFence;
		void* m_event;
		Object<ID3D12Fence> m_fence;

	public:
		CommandQueue();
		~CommandQueue();

		bool create(const D3D12_COMMAND_QUEUE_DESC &desc, ID3D12Device* device, u32 capacity);
		void destroy();

		void pushCommandList(::Graphics::CommandList* p) override;

		void execute();
		void wait();

		ID3D12CommandQueue* get() { return m_commandQueue.get(); }
	};
}