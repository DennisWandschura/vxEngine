#include "CapabilitySettingPolygonOffset.h"
#include <vxLib/gl/gl.h>

namespace Graphics
{
	CapabilitySettingPolygonOffset::CapabilitySettingPolygonOffset(F32 factor, F32 units)
		:m_factor(factor),
		m_units(units)
	{
	}

	void CapabilitySettingPolygonOffset::set() const
	{
		glPolygonOffset(m_factor, m_units);
	}
}