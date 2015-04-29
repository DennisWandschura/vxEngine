#pragma once

#include "Capability.h"

namespace Graphics
{
	class CapabilityEnableDepthTest : public Capability
	{
	public:
		CapabilityEnableDepthTest();

		void set() override;
	};
}