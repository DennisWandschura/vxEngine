/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "Decision.h"

Decision::Decision(DecisionTreeNode* trueNode, DecisionTreeNode* falseNode)
	:m_nodes(),
	m_testValue()
{
	m_nodes[0] = falseNode;
	m_nodes[1] = trueNode;
}

Decision::Decision(DecisionTreeNode* trueNode, DecisionTreeNode* falseNode, vx::Variant testValue)
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