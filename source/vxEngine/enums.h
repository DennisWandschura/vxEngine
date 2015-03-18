#ifndef __ENUMS_H
#define __ENUMS_H
#pragma once

#include <vxLib/types.h>

enum class FileType : U8 { Invalid, Mesh, Texture, Material, Scene };

enum class FileStatus : U8
{
	Exists, Loaded
};

enum class PlayerType : U32
{
	Human, AI
};

#endif