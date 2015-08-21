#pragma once

#include "Align.h"

class LookWhereYoureGoing : public Align
{
public:
	explicit LookWhereYoureGoing(EntityActor* character);

	bool getSteering(SteeringOutput* output) override;
};