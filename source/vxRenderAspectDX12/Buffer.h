#pragma once

#include <vxLib/types.h>

class Buffer
{
public:
	static const u32 ALIGNMENT = 0xffff + 1;

	static u64 calculateAllocSize(u32 size, u32 alignment = Buffer::ALIGNMENT);
};
