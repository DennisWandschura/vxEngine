#include "CapabilityEnablePolygonOffsetFill.h"
#include <vxLib/gl/StateManager.h>
#include "CapabilitySetting.h"

namespace Graphics
{
	CapabilityEnablePolygonOffsetFill::CapabilityEnablePolygonOffsetFill(CapabilitySettingPolygonOffset* setting)
		:Capability(setting)
	{
	}

	void CapabilityEnablePolygonOffsetFill::set()
	{
		vx::gl::StateManager::enable(vx::gl::Capabilities::Polygon_Offset_Fill);

		static_cast<CapabilitySettingPolygonOffset*>(m_pSetting)->set();
	}
}