#pragma once

namespace d3d
{
	class CommandQueue;
	class ShaderManager;
	class ResourceManager;
	class Device;
	class Resource;
}

#include "RootSignature.h"
#include "CommandAllocator.h"
#include "CommandList.h"
#include "PipelineState.h"
#include "DescriptorHeap.h"
#include <memory>

class DrawToBackBuffer
{
	d3d::Resource* m_layerGameTexture;
	d3d::Resource* m_layerOverlayTexture;
	d3d::PipelineState m_pipelineState;
	d3d::RootSignature m_rootSignature;
	std::unique_ptr<d3d::CommandAllocator[]> m_allocators;
	std::unique_ptr<d3d::GraphicsCommandList[]> m_commandLists;
	d3d::DescriptorHeap m_rtvHeap;
	d3d::DescriptorHeap m_srvHeap;

	bool createRootSignature(ID3D12Device* device);
	bool createPipelineState(ID3D12Device* device, d3d::ShaderManager* shaderManager);

public:
	DrawToBackBuffer();
	~DrawToBackBuffer();

	bool initialize(d3d::Device* device, d3d::ShaderManager* shaderManager, d3d::ResourceManager* resourceManager, u32 backBufferCount, u32 frameCount);
	void shutdown();

	void submitList(d3d::CommandQueue* queue, d3d::Device* device, u32 frameIndex);
};