#pragma once

#include <vxLib/gl/Base.h>
#include <memory>

namespace Graphics
{
	class CapabilitySetting;
	class Capability;

	struct CapabilityDescription
	{
		CapabilitySetting* setting;
		U8 enable;
		vx::gl::Capabilities capability;
	};

	class CapabilityFactory
	{
	public:
		static std::unique_ptr<Capability> create(const CapabilityDescription &desc);

		static std::unique_ptr<Capability> createEnablePolygonOffsetFill(F32 factor, F32 units, std::unique_ptr<CapabilitySetting>* settings);

		static std::unique_ptr<Capability> createEnableDepthTest();
		static std::unique_ptr<Capability> createDisableDepthTest();
	};
}