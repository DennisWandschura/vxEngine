#pragma once

#include <vxLib/math/Vector.h>
#include "Component.h"

namespace Component
{
	struct Render : public Base
	{
		// index into transform buffer
		U16 gpuIndex;
	};
}