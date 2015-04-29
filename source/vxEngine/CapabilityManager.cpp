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