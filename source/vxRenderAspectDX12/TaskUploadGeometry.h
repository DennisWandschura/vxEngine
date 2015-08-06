#pragma once

struct ID3D12GraphicsCommandList;
struct ID3D12Resource;
struct ID3D12CommandList;
struct ID3D12CommandAllocator;

#include <vxEngineLib/Task.h>
#include <vxLib/types.h>
#include <vector>

struct UploadTaskData
{
	ID3D12Resource* src;
	u64 srcOffset;
	ID3D12Resource* dst;
	u64 dstOffset;
	u64 size;
	u32 dstState;
};

class TaskUploadGeometry : public Task
{
	ID3D12CommandAllocator* m_cmdAllocator;
	ID3D12GraphicsCommandList* m_commandList;
	std::vector<UploadTaskData> m_data;
	std::vector<ID3D12CommandList*>* m_cmdLists;

	TaskReturnType runImpl() override;

public:
	TaskUploadGeometry(ID3D12CommandAllocator* cmdAllocator, ID3D12GraphicsCommandList* commandList, std::vector<UploadTaskData> &&data, std::vector<ID3D12CommandList*>* cmdLists);
	~TaskUploadGeometry();

	f32 getTimeMs() const override;
};