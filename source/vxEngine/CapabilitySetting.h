#pragma once

#include <vxLib/types.h>

namespace Graphics
{
	class CapabilitySetting
	{
	public:
		virtual ~CapabilitySetting(){}

		virtual void set() const = 0;
	};
}