#include "DownloadMananger.h"
#include <vxEngineLib/Graphics/CommandQueue.h>

struct DownloadManager::CopyEntry
{
	d3d::Resource* src;
	u32 srcOffset; 
	u32 size;
	u32 dstOffset;
	u8* cpuDst;
	Event evt;
};

struct DownloadManager::DownloadEntry
{
	u8* cpuDst;
	u32 size;
	u32 srcOffset;
	Event evt;
};

DownloadManager::DownloadManager()
	:m_entries(),
	m_capacity(0),
	m_size(0)
{

}

DownloadManager::~DownloadManager()
{

}

bool DownloadManager::initialize(ID3D12Device* device)
{
	if (!m_allocator.create(D3D12_COMMAND_LIST_TYPE_DIRECT, device))
		return false;

	if (!m_commandList.create(device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_allocator.get()))
		return false;

	if(!m_heapDownload.createBufferHeap(1 MBYTE, D3D12_HEAP_TYPE_READBACK, device))
		return false;

	d3d::HeapCreateBufferResourceDesc desc;
	desc.flags = D3D12_RESOURCE_FLAG_NONE;
	desc.resource = m_bufferDownload.getAddressOf();
	desc.size = 1 MBYTE;
	desc.state = D3D12_RESOURCE_STATE_COPY_DEST;
	if (!m_heapDownload.createResourceBuffer(desc))
		return false;

	m_capacity = 1 MBYTE;

	return true;
}

void DownloadManager::shutdown()
{
	m_bufferDownload.destroy();
	m_heapDownload.destroy();
	m_commandList.destroy();
	m_allocator.destroy();
}

bool DownloadManager::queueCopyBuffer(d3d::Resource* src, u32 srcOffset, u32 size, u8* cpuDst, const Event &evt)
{
	auto offset = d3d::getAlignedSize(m_size, 256u);
	auto newSize = offset + size;
	if (newSize > m_capacity)
	{

		return false;
	}

	CopyEntry copyEntry;
	copyEntry.src = src;
	copyEntry.srcOffset = srcOffset;
	copyEntry.size = size;
	copyEntry.dstOffset = offset;
	copyEntry.cpuDst = cpuDst;
	copyEntry.evt = evt;

	m_copyEntries.push_back(copyEntry);

	m_size = newSize;

	return true;
}

void DownloadManager::pushDownloadBuffer(u8* cpuDst, u32 size, d3d::Resource* src, u32 srcOffset, const Event &evt)
{
	if (!queueCopyBuffer(src, srcOffset, size, cpuDst, evt))
	{
		VX_ASSERT(false);
	}
}

void DownloadManager::submitCommandList(Graphics::CommandQueue* queue)
{
	if (!m_copyEntries.empty())
	{
		m_commandList->Reset(m_allocator.get(), nullptr);
		for (auto &it : m_copyEntries)
		{
			auto src = it.src->get();
			auto stateBefore = it.src->getOriginalState();

			m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(src, stateBefore, D3D12_RESOURCE_STATE_COPY_SOURCE));

			m_commandList->CopyBufferRegion(m_bufferDownload.get(), it.dstOffset, src, it.srcOffset, it.size);

			m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(src, D3D12_RESOURCE_STATE_COPY_SOURCE, stateBefore));

			DownloadEntry downloadEntry;
			downloadEntry.cpuDst = it.cpuDst;
			downloadEntry.evt = it.evt;
			downloadEntry.size = it.size;
			downloadEntry.srcOffset = it.srcOffset;
			m_entries.push_back(downloadEntry);
		}
		m_commandList->Close();
		m_copyEntries.clear();

		queue->pushCommandList(&m_commandList);
	}
}

void DownloadManager::downloadToCpu()
{
	if (!m_entries.empty())
	{
		D3D12_RANGE readRange;
		readRange.Begin = 0;
		readRange.End = m_size;

		u8* ptr = nullptr;
		m_bufferDownload->Map(0, &readRange, (void**)&ptr);
		for (auto &it : m_entries)
		{
			memcpy(it.cpuDst, ptr + it.srcOffset, it.size);

			it.evt.setStatus(EventStatus::Complete);
		}
		m_bufferDownload->Unmap(0, nullptr);
		m_entries.clear();

		m_size = 0;
	}
}