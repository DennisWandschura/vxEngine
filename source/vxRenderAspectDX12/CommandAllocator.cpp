#include "CommandAllocator.h"
#include <d3d12.h>

namespace d3d
{
	CommandAllocator::CommandAllocator()
		:m_commandAllocator()
	{

	}

	CommandAllocator::CommandAllocator(CommandAllocator &&rhs)
		: m_commandAllocator()
	{
		m_commandAllocator[0].swap(rhs.m_commandAllocator[0]);
		m_commandAllocator[1].swap(rhs.m_commandAllocator[1]);
	}

	CommandAllocator::~CommandAllocator()
	{

	}

	bool CommandAllocator::create(D3D12_COMMAND_LIST_TYPE type, ID3D12Device* device)
	{
		auto hr = device->CreateCommandAllocator(type, IID_PPV_ARGS(m_commandAllocator[0].getAddressOf()));
		if (hr != 0)
			return false;

		hr = device->CreateCommandAllocator(type, IID_PPV_ARGS(m_commandAllocator[1].getAddressOf()));
		return (hr == 0);
	}

	u32 CommandAllocator::reset()
	{
		m_commandAllocator[0].swap(m_commandAllocator[1]);
		auto result = m_commandAllocator[0]->Reset();
		//

		return result;
	}

	void CommandAllocator::destroy()
	{
		m_commandAllocator[0].destroy();
		m_commandAllocator[1].destroy();
	}
}