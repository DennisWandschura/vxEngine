#pragma once

#include "RenderPass.h"
#include "DescriptorHeap.h"
#include "CommandList.h"
#include "DrawIndexedIndirectCommand.h"

class RenderPassConeTrace : public RenderPass
{
	d3d::GraphicsCommandList m_commandList;
	ID3D12CommandAllocator* m_cmdAlloc;
	d3d::DescriptorHeap m_rtvHeap;
	d3d::DescriptorHeap m_srvHeap;

	bool loadShaders();
	bool createRootSignature(ID3D12Device* device);
	bool createPipelineState(ID3D12Device* device);
	bool createDescriptorHeap(ID3D12Device* device);
	void createRtv(ID3D12Device* device);
	bool createSrv(ID3D12Device* device);

public:
	explicit RenderPassConeTrace(ID3D12CommandAllocator* cmdAlloc);
	~RenderPassConeTrace();

	void getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device) override;

	bool createData(ID3D12Device* device) override;

	bool initialize(ID3D12Device* device, void* p) override;
	void shutdown() override;

	void submitCommands(Graphics::CommandQueue* queue) override;
};