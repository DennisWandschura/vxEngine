#pragma once

#include "RenderStageDescription.h"
#include "DrawCommand.h"

class RenderStage
{
	vx::uint2 m_viewportSize{0, 0};
	U32 m_frameBuffer{0};
	U32 m_clearBits{ 0 };
	U32 m_vao{0};
	U32 m_pipeline{0};
	DrawCommand m_drawCommand{};

	void setState();

public:
	RenderStage();
	RenderStage(const RenderStageDescription &desc);

	void drawArrays();
	void drawElements();
	void multiDrawElementsIndirect();
};