#pragma once

#include <vxlib/math/matrix.h>

namespace vx
{
	struct mat4d
	{
		__m256d c[4];

		void asFloat(vx::mat4* m)
		{
		}
	};
}