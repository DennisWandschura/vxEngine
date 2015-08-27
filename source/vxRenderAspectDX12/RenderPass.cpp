#include "RenderPass.h"
#include <d3d12.h>

d3d::ShaderManager* RenderPass::s_shaderManager{nullptr};
d3d::ResourceManager* RenderPass::s_resourceManager{nullptr};