#pragma once

#include "CapabilitySetting.h"

namespace Graphics
{
	class CapabilitySettingBlend : public CapabilitySetting
	{
		U16 m_function{0};
		U16 m_sfactor{0};
		U16 m_dfactor{0};

	public:
		CapabilitySettingBlend() = default;
		CapabilitySettingBlend(U16 function, U16 sFactor, U16 dFactor);

		void set() const override;
	};
}