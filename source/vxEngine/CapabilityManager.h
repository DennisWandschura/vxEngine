#pragma once

#include <vxLib/Container/sorted_vector.h>
#include <vxLib/StringID.h>
#include <memory>

namespace Graphics
{
	class Capability;
	class CapabilitySetting;

	class CapabilityManager
	{
		vx::sorted_vector<vx::StringID, std::unique_ptr<Graphics::Capability>> m_graphicsCapabilities;
		vx::sorted_vector<vx::StringID, std::unique_ptr<Graphics::CapabilitySetting>> m_graphicsCapabilitySetting;

	public:
		~CapabilityManager();

		void createCapabilitySettingsPolygonOffsetFill(F32 factor, F32 units, const char* id);

		Capability* createEnableDepthTest(const char *id);
		Capability* createDisableDepthTest(const char *id);

		Capability* createEnablePolygonOffsetFill(const char *id, const char* settingsId);
		Capability* createDisablePolygonOffsetFill(const char *id);

		Capability* getCapability(const char* id);
	};
}