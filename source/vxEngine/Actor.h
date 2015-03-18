#pragma once

#include <vxLib/StringID.h>

struct ActorFile
{
	char mesh[32];
	char material[32];
	char name[32];

	ActorFile()
	{
		mesh[0] = '\0';
		material[0] = '\0';
		name[0] = '\0';
	}
};

// describes an AI in game character
struct Actor
{
	vx::StringID64 mesh;
	vx::StringID64 material;
};