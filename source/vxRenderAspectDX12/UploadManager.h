#pragma once

struct ID3D12Resource;
struct ID3D12GraphicsCommandList;
struct ID3D12CommandAllocator;

namespace d3d
{
	class Device;
}

#include <vxLib/types.h>
#include "d3d.h"
#include "Heap.h"
#include <vector>
#include <vxLib/Allocator/StackAllocator.h>

class UploadManager
{
	struct UploadJob;

	std::vector<UploadJob> m_jobs;
	u32 m_capacity;
	u32 m_size;
	d3d::Object<ID3D12CommandAllocator> m_commandAllocator;
	d3d::Object<ID3D12GraphicsCommandList> m_commandList;
	d3d::Object<ID3D12Resource> m_uploadBuffer;
	d3d::Heap m_heap;

public:
	UploadManager();
	~UploadManager();

	bool initialize(d3d::Device* device);

	bool pushUpload(const u8* data, ID3D12Resource* dstBuffer, u32 dstOffset, u32 size, u32 state);

	ID3D12GraphicsCommandList* update();
};