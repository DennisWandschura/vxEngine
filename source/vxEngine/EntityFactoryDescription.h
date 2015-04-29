#pragma once

namespace Component
{
	struct Actor;
	struct Input;
}

struct EntityActor;
class NavGraph;

#include <vxLib/types.h>

struct EntityFactoryDescription
{
	const NavGraph* navGraph; EntityActor* entity; Component::Actor *p; Component::Input *pInput; F32 halfHeight;
};