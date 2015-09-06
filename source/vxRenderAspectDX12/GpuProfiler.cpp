#include "GpuProfiler.h"
#include <d3d12.h>
#include "CommandList.h"
#include "d3dx12.h"
#include "ResourceManager.h"

GpuProfiler::GpuProfiler()
	:m_buffer(nullptr),
	m_currentFrame(0)
{

}

GpuProfiler::~GpuProfiler()
{

}

void GpuProfiler::getRequiredMemory(u32 maxQueries, u64* bufferHeapSize)
{
	auto sizeInBytes = maxQueries * sizeof(u64) * 3;

	*bufferHeapSize = d3d::getAlignedSize(sizeInBytes, 64llu KBYTE);
}

bool GpuProfiler::initialize(u32 maxQueries, d3d::ResourceManager* resourceManager, ID3D12Device* device)
{
	D3D12_QUERY_HEAP_DESC desc;
	desc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
	desc.Count = maxQueries;
	desc.NodeMask = 1;
	auto hr = device->CreateQueryHeap(&desc, IID_PPV_ARGS(m_queryHeap.getAddressOf()));
	if (hr != 0)
		return false;

	auto sizeInBytes = maxQueries * sizeof(u64) * 3;
	sizeInBytes = d3d::getAlignedSize(sizeInBytes, 64llu KBYTE);
	m_buffer = resourceManager->createBuffer(L"queryBuffer", sizeInBytes, D3D12_RESOURCE_STATE_GENERIC_READ);
	if (m_buffer == nullptr)
		return false;

	m_countPerFrame = maxQueries;

	return true;
}

void GpuProfiler::shutdown()
{
	m_queryHeap.destroy();
}

void GpuProfiler::frame(d3d::GraphicsCommandList* cmdList)
{
	auto queryFrame = m_currentFrame - 3;
	if (queryFrame > 0)
	{
		auto queryBuffer = queryFrame % 3;
		if (m_count[queryBuffer] != 0)
		{
			auto offset = d3d::getAlignedSize(m_countPerFrame * sizeof(u64), 8llu) * queryBuffer;

			(*cmdList)->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_buffer, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));
			(*cmdList)->ResolveQueryData(m_queryHeap.get(), D3D12_QUERY_TYPE_TIMESTAMP, 0, m_count[queryBuffer], m_buffer, offset);
			(*cmdList)->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

			m_count[queryBuffer] = 0;
		}
	}

	++m_currentFrame;
}

void GpuProfiler::query(d3d::GraphicsCommandList* cmdList)
{
	auto currentBuffer = m_currentFrame % 3;

	VX_ASSERT(m_count[currentBuffer] < m_countPerFrame);
	(*cmdList)->EndQuery(m_queryHeap.get(), D3D12_QUERY_TYPE_TIMESTAMP, m_count[currentBuffer]++);
}