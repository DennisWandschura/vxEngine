#include "EngineConfig.h"
#include "yamlHelper.h"
#include "RenderAspectDescription.h"

void EngineConfig::loadFromYAML(const char* file)
{
	YAML::Node settingsFile = YAML::LoadFile(file);
	auto renderNode = settingsFile["render_settings"];

	m_resolution = renderNode["resolution"].as<vx::uint2>();
	m_shadowMapResolution = renderNode["shadow_map_resolution"].as<U32>();
	m_fov = renderNode["fov"].as<F32>();
	m_zNear = renderNode["z_near"].as<F32>();
	m_zFar = renderNode["z_far"].as<F32>();
	m_voxelGiMode = renderNode["voxel_gi_mode"].as<U32>();
	m_vsync = renderNode["vsync"].as<bool>();
	m_renderDebug = renderNode["debug"].as<bool>();
}

RenderAspectDescription EngineConfig::getRenderAspectDescription(const vx::Window* window, vx::StackAllocator* allocator) const
{
	RenderAspectDescription renderAspectDesc;
	renderAspectDesc.window = window;
	renderAspectDesc.resolution = m_resolution;
	renderAspectDesc.shadowMapResolution = m_shadowMapResolution;
	renderAspectDesc.fovRad = vx::degToRad(m_fov);
	renderAspectDesc.z_near = m_zNear;
	renderAspectDesc.z_far = m_zFar;
	renderAspectDesc.vsync = m_vsync;
	renderAspectDesc.voxelGiMode = (VoxelGiMode)m_voxelGiMode;
	renderAspectDesc.debug = m_renderDebug;
	renderAspectDesc.pAllocator = allocator;

	return renderAspectDesc;
}