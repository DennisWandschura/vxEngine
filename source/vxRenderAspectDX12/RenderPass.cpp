#include "RenderPass.h"
#include <d3d12.h>
#include "ShaderManager.h"

d3d::ShaderManager* RenderPass::s_shaderManager{nullptr};
d3d::ResourceManager* RenderPass::s_resourceManager{nullptr};
UploadManager* RenderPass::s_uploadManager{ nullptr };
vx::uint2 RenderPass::s_resolution{0, 0};
const RenderSettings* RenderPass::s_settings{ nullptr };

RenderPass::RenderPass() :m_rootSignature(), m_pipelineState() {}

RenderPass::~RenderPass()
{

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