#pragma once

#include "SmallObject.h"

class Action : public SmallObject
{
public:
	virtual void run(){}

	virtual bool isComplete() const { return true; }
};