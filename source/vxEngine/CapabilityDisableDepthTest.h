#pragma once

#include "Capability.h"

namespace Graphics
{
	class CapabilityDisableDepthTest : public Capability
	{
	public:
		CapabilityDisableDepthTest();

		void set() override;
	};
}