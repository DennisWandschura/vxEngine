#ifndef __LOADFILECALLBACK_H
#define __LOADFILECALLBACK_H
#pragma once

#include <vxLib/types.h>

enum class FileStatus : U8;
enum class FileType : U8;

#include "Variant.h"

struct LoadFileReturnType
{
	U8 result{ 0 };
	FileType type;
	FileStatus status;
};

typedef void(*LoadFileCallback)(Variant, LoadFileReturnType, void* p);
#endif