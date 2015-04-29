#pragma once

#include <vxLib/types.h>

struct FileHeader
{
	static const U32 s_magic = 0x1337b0b;

	U32 magic;
	U32 version;
	U64 crc;
};