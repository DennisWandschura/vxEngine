#include "RenderPass.h"
#include <d3d12.h>
#include "ShaderManager.h"
#include "CommandList.h"
#include "CommandAllocator.h"

d3d::ShaderManager* RenderPass::s_shaderManager{nullptr};
d3d::ResourceManager* RenderPass::s_resourceManager{nullptr};
UploadManager* RenderPass::s_uploadManager{ nullptr };
vx::uint2 RenderPass::s_resolution{0, 0};
const RenderSettings* RenderPass::s_settings{ nullptr };
GpuProfiler* RenderPass::s_gpuProfiler{nullptr};
d3d::ThreadData* RenderPass::s_threadData{nullptr};

RenderPass::RenderPass() :m_currentCommandList(nullptr), m_commandLists(), m_rootSignature(), m_pipelineState() {}

RenderPass::~RenderPass()
{
	m_commandLists.reset();
}

bool RenderPass::loadShaders(const wchar_t* const* name, u32 count)
{
	for (u32 i = 0; i < count; ++i)
	{
		auto currentName = name[i];

		if (!s_shaderManager->loadShader(currentName))
			return false;
	}

	return true;
}

bool RenderPass::createCommandLists(ID3D12Device* device, u32 type, d3d::CommandAllocator* allocators, u32 frameCount)
{
	m_commandLists = std::make_unique<d3d::GraphicsCommandList[]>(frameCount);
	for (u32 i = 0; i < frameCount; ++i)
	{
		auto allocator = allocators[i].get();
		if (!m_commandLists[i].create(device, (D3D12_COMMAND_LIST_TYPE)type, allocator, m_pipelineState.get()))
			return false;
	}

	return true;
}