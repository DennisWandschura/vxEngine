#include "Decision.h"

Decision::Decision(DecisionTreeNode* trueNode, DecisionTreeNode* falseNode)
	:m_nodes(),
	m_testValue()
{
	m_nodes[0] = falseNode;
	m_nodes[1] = trueNode;
}

Decision::Decision(DecisionTreeNode* trueNode, DecisionTreeNode* falseNode, Variant testValue)
	:m_nodes(),
	m_testValue(testValue)
{
	m_nodes[0] = falseNode;
	m_nodes[1] = trueNode;
}

TargetState* Decision::makeDecision()
{
	TargetState* state = nullptr;
	auto p = m_nodes[getBranch()];
	if (p)
	{
		state = p->makeDecision();
	}

	return state;
}