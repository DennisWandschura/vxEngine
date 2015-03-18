#include "DecisionHasDestination.h"
#include "ComponentActor.h"

DecisionHasDestination::DecisionHasDestination(const Component::Actor* pActor, DecisionTreeNode* trueNode, DecisionTreeNode* falseNode)
	:Decision(trueNode, falseNode)
{
	m_testValue.ptr = (void*)pActor;
}

U8 DecisionHasDestination::getBranch() const
{
	const Component::Actor* pActor = (const Component::Actor*)m_testValue.ptr;

	if ((pActor->flags & Component::Actor::HasDestination) == Component::Actor::HasDestination)
		return 1;

	return 0;
}