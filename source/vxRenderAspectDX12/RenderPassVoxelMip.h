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

namespace d3d
{
	class CommandAllocator;
}

#include "RenderPass.h"
#include "DescriptorHeap.h"
#include "CommandList.h"

class RenderPassVoxelMip : public RenderPass 
{
	d3d::GraphicsCommandList m_commandList;
	d3d::CommandAllocator* m_cmdAlloc;
	d3d::DescriptorHeap m_uavHeap;

	bool loadShaders();
	bool createRootSignature(ID3D12Device* device);
	bool createPipelineState(ID3D12Device* device);
	void createViews(ID3D12Device* device);

public:
	explicit RenderPassVoxelMip(d3d::CommandAllocator* cmdAlloc);
	~RenderPassVoxelMip();

	void getRequiredMemory(u64* heapSizeBuffer, u32* bufferCount, u64* heapSizeTexture, u32* textureCount, u64* heapSizeRtDs, u32* rtDsCount, ID3D12Device* device) override;

	bool createData(ID3D12Device* device) override;

	bool initialize(ID3D12Device* device, void* p) override;
	void shutdown() override;

	void buildCommands() override;
	void submitCommands(Graphics::CommandQueue* queue) override;
};