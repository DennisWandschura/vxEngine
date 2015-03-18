#include "ConditionHasPath.h"
#include "ComponentActor.h"

ConditionHasPath::ConditionHasPath(const Component::Actor* pActor)
	:m_pActor(pActor)
{
}

U8 ConditionHasPath::test() const
{
	return ((m_pActor->flags & Component::Actor::Flags::HasPath) == Component::Actor::Flags::HasPath);
}