#pragma once

#include <vxLib/types.h>

class FileInterface
{
public:
	virtual ~FileInterface(){}

	virtual const U8* loadFromMemory(const U8 *ptr, U32 version) = 0;

	virtual U64 getCrc() const = 0;

	virtual U32 getVersion() const = 0;
};