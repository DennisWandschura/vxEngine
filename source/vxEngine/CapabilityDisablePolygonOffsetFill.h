#pragma once

#include "Capability.h"

namespace Graphics
{
	class CapabilityDisablePolygonOffsetFill : public Capability
	{
	public:
		void set() override;
	};
}