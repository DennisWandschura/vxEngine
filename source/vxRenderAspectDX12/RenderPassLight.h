#pragma once

#include "RenderPass.h"

class RenderPassLight : public RenderPass
{
public:
	virtual ~RenderPassLight() {}

	virtual void setLightCount(u32 count) = 0;
};