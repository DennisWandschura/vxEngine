#pragma once

#include <vxLib/types.h>

/*
File types:
mesh
physx
scene
texture
material
shader
*/
class vxFile
{
	U32 type;
	U32 version;
	U32 size;
};