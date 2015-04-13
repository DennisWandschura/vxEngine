#pragma once

#include <vxLib/math/Vector.h>

class ScreenshotFactory
{
public:
	static void writeScreenshotToFile(const vx::uint2 &resolution, vx::float4a* data);
};