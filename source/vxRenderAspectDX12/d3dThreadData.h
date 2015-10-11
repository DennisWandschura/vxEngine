#pragma once

#include "FrameData.h"

namespace d3d
{
	struct ThreadData
	{
		FrameData m_frameData[2];
		d3d::CommandAllocator m_bundleAllocator;
		d3d::CommandAllocator m_staticAllocator;
	};
}