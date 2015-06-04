#pragma once

#include "Align.h"

class LookWhereYoureGoing : public Align
{
public:
	explicit LookWhereYoureGoing(Component::Input* character);

	bool getSteering(SteeringOutput* output) override;
};