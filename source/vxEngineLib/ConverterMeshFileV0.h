#pragma once

class ArrayAllocator;

namespace vx
{
	class MeshFile;
}

#include <vxLib/types.h>

namespace Converter
{
	class MeshFileV0
	{
	public:
		static const u8* loadFromMemory(const u8 *ptr, u32 size, ArrayAllocator* allocator, vx::MeshFile* meshFile);
		static u64 getCrc(const vx::MeshFile &meshFile);
	};
}