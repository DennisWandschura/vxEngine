#ifndef __LOADFILECALLBACK_H
#define __LOADFILECALLBACK_H
#pragma once

#include <vxLib/types.h>
#include <vxLib/Variant.h>

enum class FileStatus : U8;
enum class FileType : U8;

struct LoadFileReturnType
{
	U8 result{ 0 };
	FileType type;
	FileStatus status;
};

typedef void(*LoadFileCallback)(vx::Variant, LoadFileReturnType, void* p);
#endif