#pragma once

struct ID3D12Device;
struct ID3D12Heap;

enum D3D12_RESOURCE_FLAGS;

#include "UploadBuffer.h"

class UploadHeap
{
	ID3D12Heap* m_heap;
	u32 m_size;
	u32 m_capacity;

public:
	UploadHeap();
	~UploadHeap();

	bool create(u32 memorySizeBytes, ID3D12Device* device);
	void release();

	UploadBuffer createBuffer(u32 sizeBytes, D3D12_RESOURCE_FLAGS flags, ID3D12Device* device);
};