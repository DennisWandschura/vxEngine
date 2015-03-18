#include "DecisionHasPath.h"
#include "ComponentActor.h"

DecisionHasPath::DecisionHasPath(const Component::Actor* pActor, DecisionTreeNode* trueNode, DecisionTreeNode* falseNode)
	:Decision(trueNode, falseNode)
{
	m_testValue.ptr = (void*)pActor;
}

U8 DecisionHasPath::getBranch() const
{
	const Component::Actor* pActor = (const Component::Actor*)m_testValue.ptr;

	if ((pActor->flags & Component::Actor::HasPath) == Component::Actor::HasPath)
		return 1;

	return 0;
}