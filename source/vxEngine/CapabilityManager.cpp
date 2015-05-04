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
#include "CapabilityManager.h"
#include "Capability.h"
#include "CapabilitySetting.h"
#include "CapabilityFactory.h"
#include "CapabilitySettingPolygonOffset.h"
#include "CapabilityEnablePolygonOffsetFill.h"
#include "CapabilityDisablePolygonOffsetFill.h"

namespace Graphics
{
	CapabilityManager::~CapabilityManager()
	{

	}

	void CapabilityManager::createCapabilitySettingsPolygonOffsetFill(F32 factor, F32 units, const char* id)
	{
		auto settings = std::make_unique<CapabilitySettingPolygonOffset>(factor, units);

		m_graphicsCapabilitySetting.insert(vx::make_sid(id), std::move(settings));
	}

	Capability* CapabilityManager::createEnableDepthTest(const char *id)
	{
		auto ptr = CapabilityFactory::createEnableDepthTest();
		Capability* p = ptr.get();

		m_graphicsCapabilities.insert(vx::make_sid(id), std::move(ptr));

		return p;
	}

	Capability* CapabilityManager::createDisableDepthTest(const char *id)
	{
		auto ptr = CapabilityFactory::createDisableDepthTest();
		Capability* p = ptr.get();

		m_graphicsCapabilities.insert(vx::make_sid(id), std::move(ptr));

		return p;
	}

	Capability* CapabilityManager::createEnablePolygonOffsetFill(const char *id, const char* settingsId)
	{
		auto settingsIter = m_graphicsCapabilitySetting.find(vx::make_sid(settingsId));

		auto ptr = std::make_unique<CapabilityEnablePolygonOffsetFill>((CapabilitySettingPolygonOffset*)settingsIter->get());
		Capability* p = ptr.get();

		m_graphicsCapabilities.insert(vx::make_sid(id), std::move(ptr));

		return p;
	}

	Capability* CapabilityManager::createDisablePolygonOffsetFill(const char *id)
	{
		auto ptr = std::make_unique<CapabilityDisablePolygonOffsetFill>();
		Capability* p = ptr.get();

		m_graphicsCapabilities.insert(vx::make_sid(id), std::move(ptr));

		return p;
	}

	Capability* CapabilityManager::getCapability(const char* id)
	{
		auto sid = vx::make_sid(id);
		auto it = m_graphicsCapabilities.find(sid);

		return (it == m_graphicsCapabilities.end()) ? nullptr : it->get();
	}
}