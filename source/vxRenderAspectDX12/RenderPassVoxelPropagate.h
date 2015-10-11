#pragma once

#include "RenderPass.h"
#include "DescriptorHeap.h"

class RenderPassVoxelPropagate : public RenderPass
{
	d3d::DescriptorHeap m_srvHeap;

	bool loadShaders();
	bool createRootSignature(ID3D12Device* device);
	bool createPipelineState(ID3D12Device* device);
	bool createDescriptorHeap(ID3D12Device* device);
	void createViews(ID3D12Device* device);

public:
	RenderPassVoxelPropagate();
	~RenderPassVoxelPropagate();

	void getRequiredMemory(u64* heapSizeBuffer, u32* bufferCount, u64* heapSizeTexture, u32* textureCount, u64* heapSizeRtDs, u32* rtDsCount, ID3D12Device* device) override;

	bool createData(ID3D12Device* device) override;

	bool initialize(ID3D12Device* device, d3d::CommandAllocator* allocators, u32 frameCount) override;
	void shutdown() override;

	void buildCommands(d3d::CommandAllocator* currentAllocator, u32 frameIndex) override;
	void submitCommands(Graphics::CommandQueue* queue) override;
};