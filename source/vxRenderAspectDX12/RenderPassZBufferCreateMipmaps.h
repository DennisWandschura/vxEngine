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

#include "RenderPass.h"
#include "DescriptorHeap.h"
#include <vxLib/math/Vector.h>

class RenderPassZBufferCreateMipmaps : public RenderPass
{
	d3d::DescriptorHeap m_descriptorHeapRtv;
	d3d::DescriptorHeap m_descriptorHeapSrv;

	bool loadShaders(d3d::ShaderManager* shaderManager);
	bool createRootSignature(ID3D12Device* device);
	bool createPipelineState(ID3D12Device* device, d3d::ShaderManager* shaderManager);
	bool createViews(ID3D12Device* device, d3d::DescriptorHandleCpu* rtvHandle, d3d::DescriptorHandleCpu* srvHandle, ID3D12Resource* texture);

	void createMipMaps(ID3D12Resource* texture, d3d::DescriptorHandleCpu* rtvHandleCpu, d3d::DescriptorHandleGpu* srvHandleGpu);

public:
	RenderPassZBufferCreateMipmaps();
	~RenderPassZBufferCreateMipmaps();

	void getRequiredMemory(u64* heapSizeBuffer, u32* bufferCount, u64* heapSizeTexture, u32* textureCount, u64* heapSizeRtDs, u32* rtDsCount, ID3D12Device* device) override;

	bool createData(ID3D12Device* device) override;

	bool initialize(ID3D12Device* device, d3d::CommandAllocator* allocators, u32 frameCount) override;
	void shutdown() override;

	void buildCommands(d3d::CommandAllocator* currentAllocator, u32 frameIndex) override;
	void submitCommands(Graphics::CommandQueue* queue) override;
};