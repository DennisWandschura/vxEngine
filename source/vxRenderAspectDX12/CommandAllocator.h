#pragma once

struct ID3D12CommandAllocator;
struct ID3D12Device;

#include <vxLib/types.h>

enum class CommandAllocatorType : u32
{
	Direct = 0,
	Bundle = 1,
	Compute = 2,
	Copy = 3
};

class CommandAllocator
{
	ID3D12CommandAllocator* m_allocator;

public:
	CommandAllocator();
	~CommandAllocator();

	bool create(CommandAllocatorType type, ID3D12Device* device);
	void destroy();

	ID3D12CommandAllocator* operator->();

	ID3D12CommandAllocator* get() { return m_allocator; }
};