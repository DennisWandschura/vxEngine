#pragma once

class Scene;

#include <vxLib/StringID.h>
#include "Transform.h"

struct CreateActorData
{
	vx::Transform transform;
	vx::StringID64 mesh;
	vx::StringID64 material;
	const Scene* pScene;
	U32 index;
};