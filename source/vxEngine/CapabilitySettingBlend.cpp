#include "CapabilitySettingBlend.h"
#include <vxLib/gl/gl.h>
#include <vxLib/gl/StateManager.h>

namespace Graphics
{
	CapabilitySettingBlend::CapabilitySettingBlend(U16 function, U16 sFactor, U16 dFactor)
		:m_function(function),
		m_sfactor(sFactor),
		m_dfactor(dFactor)
	{
	}

	void CapabilitySettingBlend::set() const
	{
		glBlendEquation(m_function);
		glBlendFunc(m_sfactor, m_dfactor);
	}
}