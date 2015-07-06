#include "DefaultHeap.h"
#include <d3d12.h>
#include "d3dx12.h"
#include <algorithm>
#include <vxLib/Allocator/Allocator.h>
#include "d3dHelper.h"
#include "Buffer.h"

DefaultHeap::DefaultHeap()
	:m_heap(nullptr),
	m_size(0),
	m_capacity(0)
{

}

DefaultHeap::~DefaultHeap()
{
	release();
}

bool DefaultHeap::create(u32 memorySizeBytes, ID3D12Device* device)
{
	D3D12_HEAP_DESC desc{};
	desc.Alignment = Buffer::ALIGNMENT;
	desc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
	desc.Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
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

void DefaultHeap::release()
{
	if (m_heap)
	{
		m_heap->Release();
		m_heap = nullptr;
		m_capacity = 0;
	}
}