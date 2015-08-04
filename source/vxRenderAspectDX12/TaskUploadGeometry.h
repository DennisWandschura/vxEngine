#pragma once

struct ID3D12GraphicsCommandList;
struct ID3D12Resource;
struct ID3D12CommandList;
class CommandAllocator;

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
	CommandAllocator* m_cmdAllocator;
	ID3D12GraphicsCommandList* m_commandList;
	std::vector<UploadTaskData> m_data;
	std::vector<ID3D12CommandList*>* m_cmdLists;

public:
	TaskUploadGeometry(CommandAllocator* cmdAllocator, ID3D12GraphicsCommandList* commandList, std::vector<UploadTaskData> &&data, std::vector<ID3D12CommandList*>* cmdLists);
	~TaskUploadGeometry();

	TaskReturnType run() override;

	Task* move(vx::Allocator* allocator) override;

	f32 getTime() const override;
};