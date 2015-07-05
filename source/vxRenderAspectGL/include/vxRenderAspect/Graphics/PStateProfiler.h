#pragma once

#include <vxLib/math/Vector.h>

namespace Graphics
{
	class TextRenderer;

	class PStateProfiler
	{
		void* m_gpuHandle;
		vx::float2 m_position;

	public:
		PStateProfiler();
		~PStateProfiler();

		bool initialize(const vx::float2 &position);

		void update(TextRenderer* textRenderer);
	};
}
