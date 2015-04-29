#pragma once

#include "Capability.h"
#include "CapabilitySettingPolygonOffset.h"

namespace Graphics
{
	class CapabilityEnablePolygonOffsetFill : public Capability
	{
	public:
		explicit CapabilityEnablePolygonOffsetFill(CapabilitySettingPolygonOffset* setting);

		void set() override;
	};
}