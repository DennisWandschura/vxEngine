#pragma once

namespace d3d
{
	class CommandAllocator;
}

#include "RenderPass.h"
#include "DescriptorHeap.h"
#include "CommandList.h"

class RenderPassSSIL : public RenderPass
{
	d3d::GraphicsCommandList m_commandList;
	d3d::CommandAllocator* m_cmdAlloc;
	d3d::DescriptorHeap m_heapSrv;
	d3d::DescriptorHeap m_heapRtv;

	bool createTexture(ID3D12Device* device);
	bool loadShaders();
	bool createRootSignature(ID3D12Device* device);
	bool createPipelineState(ID3D12Device* device);
	bool createSrv(ID3D12Device* device);
	bool createRtv(ID3D12Device* device);

public:
	explicit RenderPassSSIL(d3d::CommandAllocator* cmdAlloc);
	~RenderPassSSIL();

	void getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device) override;

	bool createData(ID3D12Device* device) override;

	bool initialize(ID3D12Device* device, void* p)override;
	void shutdown() override;

	void submitCommands(Graphics::CommandQueue* queue) override;
};