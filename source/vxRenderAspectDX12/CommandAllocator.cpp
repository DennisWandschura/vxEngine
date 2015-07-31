#include "CommandAllocator.h"
#include <d3d12.h>

CommandAllocator::CommandAllocator()
	:m_allocator(nullptr)
{
}

CommandAllocator::~CommandAllocator()
{
	destroy();
}

bool CommandAllocator::create(CommandAllocatorType type, ID3D12Device* device)
{
	bool result = true;

	if (m_allocator == nullptr)
	{
		auto hresult = device->CreateCommandAllocator((D3D12_COMMAND_LIST_TYPE)type, IID_PPV_ARGS(&m_allocator));
		result = (hresult == 0);
	}

	return result;
}

void CommandAllocator::destroy()
{
	if (m_allocator != nullptr)
	{
		m_allocator->Release();
		m_allocator = nullptr;
	}
}

ID3D12CommandAllocator* CommandAllocator::operator->()
{
	return m_allocator;
}