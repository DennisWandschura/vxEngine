#include "UploadManager.h"
#include <d3d12.h>
#include "d3dx12.h"
#include "Device.h"

struct UploadManager::UploadJob
{
	ID3D12Resource* dst;
	u32 srcOffset;
	u32 dstOffset;
	u32 size;
	u32 state;
};

UploadManager::UploadManager()
	:m_jobs(),
	m_capacity(0),
	m_size(0),
	m_commandAllocator(),
	m_commandList(),
	m_uploadBuffer(),
	m_heap()
{

}

UploadManager::~UploadManager()
{

}

bool UploadManager::initialize(d3d::Device* device)
{
	auto bufferSize = d3d::getAlignedSize(10u MBYTE, 64u KBYTE);
	if (!m_heap.createBufferHeap(bufferSize, D3D12_HEAP_TYPE_UPLOAD, device))
		return false;

	if (!m_heap.createResourceBuffer(bufferSize, 0, D3D12_RESOURCE_STATE_GENERIC_READ, m_uploadBuffer.getAddressOf(), device))
		return false;

	auto hresult = device->getDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocator.getAddressOf()));
	if (hresult != 0)
		return false;

	hresult = device->getDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.get(), nullptr, IID_PPV_ARGS(m_commandList.getAddressOf()));
	if (hresult != 0)
		return false;

	m_capacity = bufferSize;

	return true;
}

bool UploadManager::pushUpload(const u8* data, ID3D12Resource* dstBuffer, u32 dstOffset, u32 size, u32 state)
{
	auto offset = m_size;
	auto alignedSize = d3d::getAlignedSize(size, 256);

	auto newSize = m_size + alignedSize;
	if (newSize >= m_capacity)
		return false;

	D3D12_RANGE range;
	range.Begin = offset;
	range.End = offset + alignedSize;

	u8* ptr = nullptr;
	auto hresult = m_uploadBuffer->Map(0, &range, (void**)&ptr);
	ptr = ptr + offset;
	memcpy(ptr, data, size);
	m_uploadBuffer->Unmap(0, &range);

	UploadJob job;
	job.dst = dstBuffer;
	job.srcOffset = offset;
	job.dstOffset = dstOffset;
	job.size = size;
	job.state = state;

	m_jobs.push_back(job);
	m_size = newSize;

	return true;
}

ID3D12GraphicsCommandList* UploadManager::update()
{
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.get(), nullptr);

	for (auto &it : m_jobs)
	{
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(it.dst, (D3D12_RESOURCE_STATES)it.state, D3D12_RESOURCE_STATE_COPY_DEST));

		m_commandList->CopyBufferRegion(it.dst, it.dstOffset, m_uploadBuffer.get(), it.srcOffset, it.size);

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(it.dst, D3D12_RESOURCE_STATE_COPY_DEST, (D3D12_RESOURCE_STATES)it.state));
	}

	auto hresult = m_commandList->Close();
	if (hresult != 0)
	{
		printf("erro upload\n");
	}

	m_size = 0;

	return m_commandList.get();
}