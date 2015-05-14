#include "ConditionActorHasPath.h"
#include "ComponentActor.h"

ConditionActorHasPath::ConditionActorHasPath(Component::ActorData* actorData)
	:m_actorData(actorData)
{

}

ConditionActorHasPath::~ConditionActorHasPath()
{

}

u8 ConditionActorHasPath::test() const
{
	if (m_actorData->path.empty())
		return 0;
	
	return 1;
}