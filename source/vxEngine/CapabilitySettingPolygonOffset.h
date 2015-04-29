#pragma once

#include "CapabilitySetting.h"

namespace Graphics
{
	class CapabilitySettingPolygonOffset : public CapabilitySetting
	{
		F32 m_factor{ 0.0f };
		F32 m_units{ 0.0f };

	public:
		CapabilitySettingPolygonOffset() = default;
		CapabilitySettingPolygonOffset(F32 factor, F32 units);

		void set() const override;
	};
}