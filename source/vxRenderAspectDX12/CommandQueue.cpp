#include "CommandQueue.h"
#include <d3d12.h>
#include "CommandList.h"

namespace d3d
{
	CommandQueue::CommandQueue()
		:m_lists(),
		m_listCount(0),
		m_listCapacity(0),
		m_commandQueue(),
		m_currentFence(0),
		m_event(nullptr),
		m_fence()
	{

	}

	CommandQueue::~CommandQueue()
	{

	}

	bool CommandQueue::create(const D3D12_COMMAND_QUEUE_DESC &desc, ID3D12Device* device, u32 capacity)
	{
		auto hresult = device->CreateCommandQueue(&desc, IID_PPV_ARGS(m_commandQueue.getAddressOf()));
		if (hresult != 0)
			return false;

		hresult = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.getAddressOf()));
		if (hresult != 0)
			return false;

		m_event = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);

		m_lists = std::make_unique<ID3D12CommandList*[]>(capacity);
		m_listCapacity = capacity;

		return true;
	}

	void CommandQueue::destroy()
	{
		CloseHandle(m_event);
		m_fence.destroy();
		m_commandQueue.destroy();

		m_event = nullptr;
	}

	void CommandQueue::pushCommandList(::Graphics::CommandList* p)
	{
		VX_ASSERT(m_listCount < m_listCapacity);
		VX_ASSERT(p->getApiType() == ::Graphics::CommandApiType::D3D);
		auto ptr = (d3d::GraphicsCommandList*)p;

		m_lists[m_listCount++] = ptr->get();
	}

	void CommandQueue::execute()
	{
		m_commandQueue->ExecuteCommandLists(m_listCount, m_lists.get());
		m_listCount = 0;
	}

	void CommandQueue::wait()
	{
		const u64 fence = m_currentFence++;
		m_commandQueue->Signal(m_fence.get(), fence);

		if (m_fence->GetCompletedValue() < fence)
		{
			m_fence->SetEventOnCompletion(fence, m_event);
			WaitForSingleObject(m_event, INFINITE);
		}
	}
}