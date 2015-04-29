#pragma once

namespace vx
{
	class PoolAllocator;
}

struct EntityFactoryDescription;

#include <vxLib/types.h>

struct ActorFactory
{
	static void create(const EntityFactoryDescription &description, vx::PoolAllocator* pAllocator);
};