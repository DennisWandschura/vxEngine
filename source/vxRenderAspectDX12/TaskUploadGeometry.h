#pragma once

/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

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
	std::mutex* m_mutexCmdList;

	TaskReturnType runImpl() override;

public:
	TaskUploadGeometry(ID3D12CommandAllocator* cmdAllocator, ID3D12GraphicsCommandList* commandList, std::vector<UploadTaskData> &&data, std::vector<ID3D12CommandList*>* cmdLists, std::mutex* mutex);
	~TaskUploadGeometry();

	f32 getTimeMs() const override;
};