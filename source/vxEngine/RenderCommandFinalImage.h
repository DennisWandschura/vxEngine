#pragma once

#include "RenderCommand.h"

class RenderCommandFinalImage : public RenderCommand
{
public:
	void render(U32 count) override;
};