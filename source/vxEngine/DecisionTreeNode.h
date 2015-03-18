#pragma once

class TargetState;

#include "SmallObject.h"

class DecisionTreeNode : public SmallObject
{
public:
	virtual ~DecisionTreeNode(){}
	virtual TargetState* makeDecision() = 0;
};