#include "CommandAllocator.h"
#include <d3d12.h>

namespace d3d
{
	CommandAllocator::CommandAllocator()
		:m_commandAllocator()
	{

	}

	CommandAllocator::CommandAllocator(CommandAllocator &&rhs)
		: m_commandAllocator(std::move(rhs.m_commandAllocator))
	{

	}

	CommandAllocator::~CommandAllocator()
	{

	}

	bool CommandAllocator::create(D3D12_COMMAND_LIST_TYPE type, ID3D12Device* device)
	{
		auto hr = device->CreateCommandAllocator(type, IID_PPV_ARGS(m_commandAllocator.getAddressOf()));
		return (hr == 0);
	}

	void CommandAllocator::destroy()
	{
		m_commandAllocator.destroy();
	}
}