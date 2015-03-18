#pragma once

#pragma once

namespace Component
{
	struct Actor;
}

#include "Decision.h"

class DecisionHasPath : public Decision
{
	U8 getBranch() const;

public:
	DecisionHasPath(const Component::Actor* pActor, DecisionTreeNode* trueNode, DecisionTreeNode* falseNode);
};