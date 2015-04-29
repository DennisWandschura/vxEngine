#include "CapabilityFactory.h"
#include "CapabilityDisableBlend.h"
#include "CapabilityDisablePolygonOffsetFill.h"
#include "CapabilityEnableBlend.h"
#include "CapabilityEnablePolygonOffsetFill.h"
#include "CapabilityEnableDepthTest.h"
#include "CapabilityDisableDepthTest.h"

namespace Graphics
{
	std::unique_ptr<Capability> CapabilityFactory::create(const CapabilityDescription &desc)
	{
		std::unique_ptr<Capability> ptr;
		if (desc.enable == 0)
		{
			switch (desc.capability)
			{
			case vx::gl::Capabilities::Blend:
				ptr = std::make_unique<Graphics::CapabilityDisableBlend>();
				break;
			case vx::gl::Capabilities::Depth_Test:
				ptr = createDisableDepthTest();
				break;
			case vx::gl::Capabilities::Polygon_Offset_Fill:
				ptr = std::make_unique<Graphics::CapabilityDisablePolygonOffsetFill>();
				break;
			default:
				break;
			}
		}
		else
		{
			switch (desc.capability)
			{
			case vx::gl::Capabilities::Blend:
				ptr = std::make_unique<Graphics::CapabilityEnableBlend>((Graphics::CapabilitySettingBlend*)desc.setting);
				break;
			case vx::gl::Capabilities::Depth_Test:
				ptr = createEnableDepthTest();
				break;
			case vx::gl::Capabilities::Polygon_Offset_Fill:
				ptr = std::make_unique<Graphics::CapabilityEnablePolygonOffsetFill>((Graphics::CapabilitySettingPolygonOffset*)desc.setting);
				break;
			default:
				break;
			}
		}

		return ptr;
	}

	std::unique_ptr<Capability> CapabilityFactory::createEnablePolygonOffsetFill(F32 factor, F32 units, std::unique_ptr<CapabilitySetting>* settings)
	{
		*settings = std::make_unique<Graphics::CapabilitySettingPolygonOffset>(factor, units);

		return std::make_unique<Graphics::CapabilityEnablePolygonOffsetFill>((Graphics::CapabilitySettingPolygonOffset*)settings);
	}

	std::unique_ptr<Capability> CapabilityFactory::createEnableDepthTest()
	{
		return std::make_unique<Graphics::CapabilityEnableDepthTest>();
	}

	std::unique_ptr<Capability> CapabilityFactory::createDisableDepthTest()
	{
		return std::make_unique<Graphics::CapabilityDisableDepthTest>();
	}
}