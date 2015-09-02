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
	class ShaderManager;
}

struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct ID3D12Resource;
struct ID3D12CommandSignature;

#include "RenderPass.h"
#include <vxLib/math/Vector.h>
#include "DescriptorHeap.h"
#include <memory>

class GBufferRenderer : public RenderPass
{
	struct ColdData;

	d3d::Object<ID3D12GraphicsCommandList> m_commandList;
	ID3D12CommandAllocator* m_cmdAlloc;
	d3d::DescriptorHeap m_descriptorHeapBuffers;
	d3d::DescriptorHeap m_descriptorHeapRt;
	d3d::DescriptorHeap m_descriptorHeapDs;
	d3d::Object<ID3D12CommandSignature> m_commandSignature;
	u32 m_countOffset;
	u32 m_drawCount;
	std::unique_ptr<ColdData> m_coldData;

	void createTextureDescriptions();

	bool loadShaders(d3d::ShaderManager* shaderManager);
	bool createRootSignature(ID3D12Device* device);
	bool createPipelineState(ID3D12Device* device, d3d::ShaderManager* shaderManager);
	bool createTextures(d3d::ResourceManager* resourceManager, const vx::uint2 &resolution, ID3D12Device* device);
	bool createDescriptorHeap(ID3D12Device* device);

	void createBufferViews(d3d::ResourceManager* resourceManager, ID3D12Device* device);

public:
	GBufferRenderer(ID3D12CommandAllocator* cmdAlloc, u32 countOffset);
	~GBufferRenderer();

	void getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device) override;

	bool createData(ID3D12Device* device) override;

	bool initialize(ID3D12Device* device, void* p) override;
	void shutdown() override;

	void setDrawCount(u32 count) { m_drawCount = count; }

	void submitCommands(ID3D12CommandList** list, u32* index) override;
};