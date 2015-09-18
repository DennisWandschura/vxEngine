#pragma once

namespace d3d
{
	class CommandAllocator;
}

#include "RenderPassLight.h"
#include "DescriptorHeap.h"
#include "CommandList.h"

class RenderPassShading : public RenderPassLight
{
	d3d::GraphicsCommandList m_commandList;
	d3d::CommandAllocator* m_cmdAlloc;
	d3d::DescriptorHeap m_heapSrv;
	d3d::DescriptorHeap m_heapRtv;
	d3d::DescriptorHeap m_heapDsv;
	u32 m_lightCount;

	bool createTexture(ID3D12Device* device);
	bool loadShaders();
	bool createRootSignature(ID3D12Device* device);
	bool createPipelineState(ID3D12Device* device);
	bool createSrv(ID3D12Device* device);
	bool createRtv(ID3D12Device* device);

public:
	explicit RenderPassShading(d3d::CommandAllocator* cmdAlloc);
	~RenderPassShading();

	void getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device) override;

	bool createData(ID3D12Device* device) override;

	bool initialize(ID3D12Device* device, void* p)override;
	void shutdown() override;

	void submitCommands(Graphics::CommandQueue* queue) override;

	void setLightCount(u32 count) override { m_lightCount = count; }
};