#include "CommandList.h"
#include <d3d12.h>

namespace d3d
{
	GraphicsCommandList::GraphicsCommandList()
		: ::Graphics::CommandList(::Graphics::CommandApiType::D3D),
		m_list()
	{

	}

	GraphicsCommandList::GraphicsCommandList(GraphicsCommandList &&rhs)
		: ::Graphics::CommandList(std::move(rhs)),
		m_list(std::move(rhs.m_list))
	{

	}

	GraphicsCommandList::~GraphicsCommandList()
	{
		
	}

	bool GraphicsCommandList::create(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator *pCommandAllocator, ID3D12PipelineState *pInitialState, u32 nodeMask)
	{
		bool result = true;

		if (m_list.get() == nullptr)
		{
			auto hr = device->CreateCommandList(nodeMask, type, pCommandAllocator, pInitialState, IID_PPV_ARGS(m_list.getAddressOf()));
			result = (hr == 0);

			if (result)
			{
				m_list->Close();
			}
		}

		return result;
	}

	void GraphicsCommandList::destroy()
	{
		m_list.destroy();
	}

	ID3D12GraphicsCommandList* GraphicsCommandList::operator->()
	{
		return m_list.get();
	}
}