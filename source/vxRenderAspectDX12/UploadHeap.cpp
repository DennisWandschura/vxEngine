#include "UploadHeap.h"
#include <d3d12.h>
#include "d3dx12.h"
#include <algorithm>
#include <vxLib/Allocator/Allocator.h>
#include "d3dHelper.h"

UploadHeap::UploadHeap()
	:m_heap(nullptr),
	m_size(0),
	m_capacity(0)
{

}

UploadHeap::~UploadHeap()
{
	release();
}

bool UploadHeap::create(u32 memorySizeBytes, ID3D12Device* device)
{
	D3D12_HEAP_DESC desc{};
	desc.Alignment = Buffer::ALIGNMENT;
	desc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
	desc.Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	desc.SizeInBytes = Buffer::calculateAllocSize(memorySizeBytes);

	auto result = device->CreateHeap(&desc, IID_PPV_ARGS(&m_heap));
	if (result != 0)
	{
		return false;
	}

	m_capacity = memorySizeBytes;
	m_size = 0;

	return true;
}

void UploadHeap::release()
{
	if (m_heap)
	{
		m_heap->Release();
		m_heap = nullptr;
		m_capacity = 0;
	}
}

UploadBuffer UploadHeap::createBuffer(u32 sizeBytes, D3D12_RESOURCE_FLAGS flags, ID3D12Device* device)
{
	ID3D12Resource* resource = nullptr;

	auto size = m_size;
	auto offset = Buffer::calculateAllocSize(size);
	auto allocSize = Buffer::calculateAllocSize(sizeBytes);

	auto newSize = size + allocSize;
	if (newSize <= m_capacity)
	{
		auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(allocSize, flags);

		auto error = device->CreatePlacedResource(m_heap, offset, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));
		if (error == 0)
		{
			m_size = newSize;
		}
		else
		{
			printError(error);
		}
	}

	return UploadBuffer(resource, allocSize);
}

