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
#include <vxEngineLib/EngineConfig.h>
#include <vxEngineLib/ParserNode.h>
#include <thread>

EngineConfig g_engineConfig{};

namespace EngineConfigCpp
{
	void loadTextureSettings(const Parser::Node* rendererSettingsNode, Graphics::TextureSettings* textureSettings)
	{
		auto textureSettingsNode = rendererSettingsNode->get("texture");
		u32 texMode = 0;
		textureSettingsNode->get("mode")->as(&texMode);

		u32 texDim = 0;
		textureSettingsNode->get("dim")->as(texMode, &texDim);

		textureSettings->m_dim = texDim;
	}

	void loadShadowSettings(const Parser::Node* rendererSettingsNode, Graphics::ShadowRendererSettings* shadowSettings, u8* shadowMode)
	{
		auto shadowSettingsNode = rendererSettingsNode->get("shadow");

		u32 shadowModeValue;
		shadowSettingsNode->get("mode")->as(&shadowModeValue);
		if (shadowModeValue != 0)
		{
			u32 index = shadowModeValue - 1;

			shadowSettingsNode->get("resolution")->as(index, &shadowSettings->m_shadowMapResolution);
			shadowSettingsNode->get("lights")->as(index, &shadowSettings->m_maxShadowCastingLights);
		}

		*shadowMode = shadowModeValue;
	}

	void loadVoxelSettings(const Parser::Node* rendererSettingsNode, Graphics::VoxelRendererSettings* voxelSettings, u8* voxelMode)
	{
		auto voxelSettingsNode = rendererSettingsNode->get("voxel_gi");

		u32 voxelModeValue = 0;

		voxelSettingsNode->get("mode")->as(&voxelModeValue);
		voxelSettingsNode->get("resolution")->as(0, &voxelSettings->m_voxelTextureSize);
		voxelSettingsNode->get("grid")->as(0, &voxelSettings->m_voxelGridDim);

		*voxelMode = voxelModeValue;
	}
}

bool EngineConfig::loadFromFile(const char* file)
{
	if (!m_root.createFromFile(file))
		return false;

	m_root.get("resolution")->as(&m_resolution);
	m_root.get("fov")->as(&m_fovDeg);
	m_root.get("z_near")->as(&m_zNear);
	m_root.get("z_far")->as(&m_zFar);
	m_root.get("vsync")->as(&m_vsync);
	m_root.get("debug")->as(&m_renderDebug);

	m_rendererSettings.m_renderMode = Graphics::RendererSettings::Mode_GL;
	auto rendererSettings = m_root.get("renderer");
	auto renderMode = rendererSettings->get("mode");
	if (renderMode)
	{
		u32 value = 0;
		renderMode->as(&value);

		m_rendererSettings.m_renderMode = (Graphics::RendererSettings::Mode)value;
	}

	EngineConfigCpp::loadTextureSettings(rendererSettings, &m_rendererSettings.m_textureSettings);
	EngineConfigCpp::loadShadowSettings(rendererSettings, &m_rendererSettings.m_shadowSettings, &m_rendererSettings.m_shadowMode);

	rendererSettings->get("maxMeshInstances")->as(&m_rendererSettings.m_maxMeshInstances);
	rendererSettings->get("maxActiveLights")->as(&m_rendererSettings.m_maxActiveLights);

	EngineConfigCpp::loadVoxelSettings(rendererSettings, &m_rendererSettings.m_voxelSettings, &m_rendererSettings.m_voxelGIMode);

	m_threads = std::thread::hardware_concurrency();

	return true;
}