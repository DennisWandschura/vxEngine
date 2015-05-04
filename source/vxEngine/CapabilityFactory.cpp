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