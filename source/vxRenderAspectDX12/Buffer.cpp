#include "Buffer.h"

u64 Buffer::calculateAllocSize(u32 size, u32 alignment)
{
	auto allocDiv = (size + alignment - 1) / alignment;
	auto allocSize = alignment * allocDiv;

	return allocSize;
}