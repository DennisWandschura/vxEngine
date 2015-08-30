#pragma once

#include "RenderPass.h"
#include "CommandList.h"
#include "DescriptorHeap.h"

class RenderPassCullLights : public RenderPass
{
	d3d::GraphicsCommandList m_commandList;
	ID3D12CommandAllocator* m_allocator;
	d3d::DescriptorHeap m_rvHeap;
	d3d::Object<ID3D12DescriptorHeap> m_rtvHeap;
	d3d::Object<ID3D12DescriptorHeap> m_dsvHeap;
	u32 m_lightCount;

	bool loadShaders();
	bool createRootSignature(ID3D12Device* device);
	bool createPipelineState(ID3D12Device* device);
	bool createTexture(ID3D12Device* device);
	bool createBuffer();
	bool createViews(ID3D12Device* device);
	bool createRtvDsv(ID3D12Device* device);

public:
	explicit RenderPassCullLights(ID3D12CommandAllocator* allocator);
	~RenderPassCullLights();

	void getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device) override;

	bool initialize(ID3D12Device* device, void* p) override;
	void shutdown() override;

	void submitCommands(ID3D12CommandList** list, u32* index) override;

	void setLightCount(u32 count) { m_lightCount = count; }
};