#pragma once

#include "Capability.h"
#include "CapabilitySettingBlend.h"

namespace Graphics
{
	class CapabilityEnableBlend : public Capability
	{
	public:
		explicit CapabilityEnableBlend(CapabilitySettingBlend* setting);

		void set() override;
	};
}