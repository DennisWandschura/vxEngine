#include "Buffer.h"

u64 Buffer::calculateAllocSize(u32 size)
{
	auto allocDiv = (size + Buffer::ALIGNMENT - 1) / Buffer::ALIGNMENT;
	auto allocSize = Buffer::ALIGNMENT * allocDiv;

	return allocSize;
}