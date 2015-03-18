#include "ConditionHasDestination.h"
#include "ComponentActor.h"

ConditionHasDestination::ConditionHasDestination(const Component::Actor* pActor)
	:m_pActor(pActor)
{

}

U8 ConditionHasDestination::test() const
{
	return ((m_pActor->flags & Component::Actor::HasDestination) == Component::Actor::HasDestination);
}

ConditionHasNoDestination::ConditionHasNoDestination(const Component::Actor* pActor)
	:m_pActor(pActor)
{
}

U8 ConditionHasNoDestination::test() const
{
	return ((m_pActor->flags & Component::Actor::HasDestination) != Component::Actor::HasDestination);
}