#include "CapabilityEnableBlend.h"
#include <vxLib/gl/StateManager.h>
#include "CapabilitySetting.h"

namespace Graphics
{
	CapabilityEnableBlend::CapabilityEnableBlend(CapabilitySettingBlend* setting)
		:Capability(setting)
	{
	}

	void CapabilityEnableBlend::set()
	{
		vx::gl::StateManager::enable(vx::gl::Capabilities::Blend);

		static_cast<CapabilitySettingBlend*>(m_pSetting)->set();
	}
}