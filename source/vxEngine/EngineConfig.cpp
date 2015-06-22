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
#include "RenderAspectDescription.h"
#include <vxEngineLib/ParserNode.h>
#include <thread>

EngineConfig g_engineConfig{};

bool EngineConfig::loadFromFile(const char* file)
{
	m_root.createFromFile(file);

	m_root.get("resolution")->as(&m_resolution);
	m_root.get("fov")->as(&m_fov);
	m_root.get("z_near")->as(&m_zNear);
	m_root.get("z_far")->as(&m_zFar);
	m_root.get("voxel_gi_mode")->as(&m_rendererSettings.m_voxelGIMode);
	m_root.get("vsync")->as(&m_vsync);
	m_root.get("debug")->as(&m_renderDebug);

	auto rendererSettings = m_root.get("renderer");
	u32 shadowMode;
	rendererSettings->get("shadowMode")->as(&shadowMode);

	m_rendererSettings.m_shadowMode = shadowMode;

	switch (shadowMode)
	{
	case 0:
	{
	}break;
	case 1:
	{
		auto shadowSettings = rendererSettings->get("shadow_low");
		shadowSettings->get("shadow_map_resolution")->as(&m_rendererSettings.m_shadowSettings.m_shadowMapResolution);
		shadowSettings->get("maxShadowCastingLights")->as(&m_rendererSettings.m_shadowSettings.m_maxShadowCastingLights);
	}break;
	case 2:
	{
		auto shadowSettings = rendererSettings->get("shadow_medium");
		shadowSettings->get("shadow_map_resolution")->as(&m_rendererSettings.m_shadowSettings.m_shadowMapResolution);
		shadowSettings->get("maxShadowCastingLights")->as(&m_rendererSettings.m_shadowSettings.m_maxShadowCastingLights);
	}break;
	case 3:
	{
		auto shadowSettings = rendererSettings->get("shadow_high");
		shadowSettings->get("shadow_map_resolution")->as(&m_rendererSettings.m_shadowSettings.m_shadowMapResolution);
		shadowSettings->get("maxShadowCastingLights")->as(&m_rendererSettings.m_shadowSettings.m_maxShadowCastingLights);
	}break;
	default:
		break;
	}

	rendererSettings->get("maxMeshInstances")->as(&m_rendererSettings.m_maxMeshInstances);
	rendererSettings->get("maxActiveLights")->as(&m_rendererSettings.m_maxActiveLights);


	m_threads = std::thread::hardware_concurrency();

	return true;
}

RenderAspectDescription EngineConfig::getRenderAspectDescription(const vx::Window* window, vx::StackAllocator* allocator) const
{
	RenderAspectDescription renderAspectDesc;
	renderAspectDesc.window = window;
	renderAspectDesc.resolution = m_resolution;
	renderAspectDesc.fovRad = vx::degToRad(m_fov);
	renderAspectDesc.z_near = m_zNear;
	renderAspectDesc.z_far = m_zFar;
	renderAspectDesc.vsync = m_vsync;
	renderAspectDesc.debug = m_renderDebug;
	renderAspectDesc.pAllocator = allocator;

	return renderAspectDesc;
}