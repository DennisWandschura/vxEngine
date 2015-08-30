#pragma once

struct ID3D12CommandQueue;
struct D3D12_COMMAND_QUEUE_DESC;
struct ID3D12CommandList;
struct ID3D12Fence;

#include "d3d.h"

namespace d3d
{
	class CommandQueue
	{
		Object<ID3D12CommandQueue> m_commandQueue;
		u64 m_currentFence;
		void* m_event;
		Object<ID3D12Fence> m_fence;

	public:
		CommandQueue();
		~CommandQueue();

		bool create(const D3D12_COMMAND_QUEUE_DESC &desc, ID3D12Device* device);
		void destroy();

		void execute(u32 count, ID3D12CommandList* const * lists);
		void wait();

		ID3D12CommandQueue* get() { return m_commandQueue.get(); }
	};
}