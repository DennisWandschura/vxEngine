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