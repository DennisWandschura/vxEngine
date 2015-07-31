#include "TaskUploadGeometry.h"
#include <d3d12.h>
#include "CommandAllocator.h"
#include "d3dHelper.h"

TaskUploadGeometry::TaskUploadGeometry(u32 tid, CommandAllocator* cmdAllocator, ID3D12GraphicsCommandList* commandList, std::vector<UploadTaskData> &&data, 
	std::vector<ID3D12CommandList*>* cmdLists)
	:Task(tid),
	m_cmdAllocator(cmdAllocator), 
	m_commandList(commandList),
	m_data(std::move(data)),
	m_cmdLists(cmdLists)
{

}

TaskUploadGeometry::~TaskUploadGeometry()
{
}

TaskReturnType TaskUploadGeometry::run()
{
	auto allocator = m_cmdAllocator->get();
	auto hresult = allocator->Reset();
	if (hresult != 0)
	{
		printf("error allocator->reset()\n");
	}

	hresult = m_commandList->Reset(allocator, nullptr);
	if (hresult != 0)
	{
		printf("error m_commandList->reset()\n");
	}

	for (auto &it : m_data)
	{
		setResourceBarrier(m_commandList, it.dst, (D3D12_RESOURCE_STATES)it.dstState, D3D12_RESOURCE_STATE_COPY_DEST);
		//setResourceBarrier(m_commandList, it.src, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_SOURCE);

		m_commandList->CopyBufferRegion(it.dst, it.dstOffset, it.src, it.srcOffset, it.size);

		//setResourceBarrier(m_commandList, it.src, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ);
		setResourceBarrier(m_commandList, it.dst, D3D12_RESOURCE_STATE_COPY_DEST, (D3D12_RESOURCE_STATES)it.dstState);
	}

	hresult = m_commandList->Close();
	if (hresult != 0)
	{
		printf("error m_commandList->close()\n");
	}

	m_data.clear();

	m_cmdLists->push_back(m_commandList);
	return TaskReturnType::Success;
}

Task* TaskUploadGeometry::move(vx::Allocator* allocator)
{
	return nullptr;
}