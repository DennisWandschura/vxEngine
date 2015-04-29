#pragma once

namespace vx
{
	class PoolAllocator;
}

struct EntityFactoryDescription;

class EntityFactory
{
public:
	static void create(const EntityFactoryDescription &desc, vx::PoolAllocator* pAllocator);
};