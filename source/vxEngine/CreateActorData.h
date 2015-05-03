#pragma once

class Scene;

#include <vxLib/StringID.h>
#include "Transform.h"

struct CreateActorData
{
	vx::Transform transform;
	vx::StringID mesh;
	vx::StringID material;
	const Scene* pScene;
	U32 index;
};