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

class DownloadManager;

namespace d3d
{
	class CommandAllocator;
}

#include "RenderPass.h"
#include "CommandList.h"
#include "DescriptorHeap.h"
#include <vxEngineLib/Event.h>

class RenderPassCullLights : public RenderPass
{
	d3d::GraphicsCommandList m_commandList;
	d3d::CommandAllocator* m_allocator;
	d3d::DescriptorHeap m_rvHeap;
	d3d::Object<ID3D12DescriptorHeap> m_dsvHeap;
	d3d::Object<ID3D12PipelineState> m_pipelineStateZeroLights;
	d3d::Object<ID3D12RootSignature> m_rootSignatureZeroLights;
	u32 m_lightCount;
	Event m_checkEvent;
	Event m_downloadEvent;
	DownloadManager* m_downloadManager;
	u8* m_cpuDst;

	bool loadShaders();
	bool createRootSignature(ID3D12Device* device);
	bool createRootSignatureZero(ID3D12Device* device);
	bool createPipelineState(ID3D12Device* device);
	bool createPipelineStateZero(ID3D12Device* device);
	bool createViews(ID3D12Device* device);
	bool createRtvDsv(ID3D12Device* device);

public:
	RenderPassCullLights(d3d::CommandAllocator* allocator, DownloadManager* downloadManager);
	~RenderPassCullLights();

	void getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device) override;

	bool createData(ID3D12Device* device) override;

	bool initialize(ID3D12Device* device, void* p) override;
	void shutdown() override;

	void submitCommands(Graphics::CommandQueue* queue) override;

	void setLightCount(u32 count) { m_lightCount = count; }
	void setEvent(const Event &checkEvt, const Event &dlEvent, u8* cpuDst);
};