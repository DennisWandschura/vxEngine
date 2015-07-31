#pragma once

struct ID3D12Heap;
struct ID3D12Device;

#include <vxLib/types.h>

class DefaultHeap
{
	ID3D12Heap* m_heap;
	u32 m_size;
	u32 m_capacity;

public:
	DefaultHeap();
	~DefaultHeap();

	bool create(u32 memorySizeBytes, ID3D12Device* device);
	void release();

	ID3D12Heap* get() { return m_heap; }
};
