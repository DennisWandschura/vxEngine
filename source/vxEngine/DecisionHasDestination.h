#pragma once

namespace Component
{
	struct Actor;
}

#include "Decision.h"

class DecisionHasDestination : public Decision
{
	U8 getBranch() const;

public:
	DecisionHasDestination(const Component::Actor* pActor, DecisionTreeNode* trueNode, DecisionTreeNode* falseNode);
};