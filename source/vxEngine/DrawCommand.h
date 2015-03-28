#pragma once

#include "DrawCommandDescription.h"

class DrawCommand
{
	U32 m_indirectBuffer{ 0 };
	U32 m_count{ 0 };
	U32 m_dataType{ 0 };
	U32 m_mode{ 0 };

	U32 getMode(DrawCommandDescription::Mode mode);

public:
	DrawCommand() = default;
	DrawCommand(const DrawCommandDescription &desc);

	void drawArrays();
	void drawElements();
	void multiDrawIndirect();
};