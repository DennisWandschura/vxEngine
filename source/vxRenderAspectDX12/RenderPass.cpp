#include "RenderPass.h"
#include <d3d12.h>

d3d::ShaderManager* RenderPass::s_shaderManager{nullptr};
d3d::ResourceManager* RenderPass::s_resourceManager{nullptr};
UploadManager* RenderPass::s_uploadManager{ nullptr };
vx::uint2 RenderPass::s_resolution{0, 0};

RenderPass::RenderPass() :m_rootSignature(), m_pipelineState() {}

RenderPass::~RenderPass()
{

}