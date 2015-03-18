#pragma once

#include <vxLib/types.h>
#include "SmallObject.h"

class Condition : public SmallObject
{
public:
	virtual ~Condition(){}
	virtual U8 test() const = 0;
};