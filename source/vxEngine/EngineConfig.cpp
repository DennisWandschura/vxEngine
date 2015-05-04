/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
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