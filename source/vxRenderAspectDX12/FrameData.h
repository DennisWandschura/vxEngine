#pragma once

#include <vxLib/types.h>
#include "CommandAllocator.h"
#include "CommandList.h"

struct FrameData
{
	d3d::CommandAllocator m_allocatorUpload;
	d3d::CommandAllocator m_allocatorCopy;
	d3d::CommandAllocator m_allocatorDownload;
	d3d::CommandAllocator m_allocatorProfiler;
	d3d::GraphicsCommandList m_commandListProfiler;
	u64 m_fenceValue;
};