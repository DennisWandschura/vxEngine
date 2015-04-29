#include "EntityFactory.h"
#include "ActorFactory.h"

void EntityFactory::create(const EntityFactoryDescription &description, vx::PoolAllocator* pAllocator)
{
	ActorFactory::create(description, pAllocator);
}