#pragma once

#include "Action.h"

class ActionUseEntity : public Action
{
public:
	virtual ~ActionUseEntity() {}

	virtual void startUse() = 0;
	virtual void stopUse() = 0;
};