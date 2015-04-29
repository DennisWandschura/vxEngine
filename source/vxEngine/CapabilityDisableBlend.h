#pragma once

#include "Capability.h"

namespace Graphics
{
	class CapabilityDisableBlend : public Capability
	{
	public:
		void set() override;
	};
}