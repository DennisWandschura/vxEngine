#include "CommandQueue.h"
#include <d3d12.h>
#include "CommandList.h"

namespace d3d
{
	CommandQueue::CommandQueue()
		:m_lists(),
		m_listCount(0),
		m_listCapacity(0),
		m_commandQueue()
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

		m_lists = std::make_unique<ID3D12CommandList*[]>(capacity);
		m_listCapacity = capacity;

		return true;
	}

	void CommandQueue::destroy()
	{
		m_commandQueue.destroy();
	}

	void CommandQueue::pushCommandList(::Graphics::CommandList* p)
	{
		if (p)
		{
			VX_ASSERT(m_listCount < m_listCapacity);
			VX_ASSERT(p->getApiType() == ::Graphics::CommandApiType::D3D);
			auto ptr = (d3d::GraphicsCommandList*)p;

			m_lists[m_listCount++] = ptr->get();
		}
	}

	void CommandQueue::execute()
	{
		m_commandQueue->ExecuteCommandLists(m_listCount, m_lists.get());
		m_listCount = 0;
	}

	void CommandQueue::signal(ID3D12Fence* fence, u64 fenceValue)
	{
		m_commandQueue->Signal(fence, fenceValue);
	}
}