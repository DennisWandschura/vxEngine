#pragma once

#include "DecisionTreeNode.h"
#include <vxLib/types.h>
#include "Variant.h"

class Decision : public DecisionTreeNode
{
protected:
	DecisionTreeNode* m_nodes[2];
	Variant m_testValue;

	virtual U8 getBranch() const = 0;

public:
	Decision(DecisionTreeNode* trueNode, DecisionTreeNode* falseNode);
	Decision(DecisionTreeNode* trueNode, DecisionTreeNode* falseNode, Variant testValue);

	TargetState* makeDecision() override;
};