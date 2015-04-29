#pragma once

#include <vector>
#include "RenderPass.h"

namespace Graphics
{
	class Frame
	{
		std::vector<RenderPass> m_renderPasses;

	public:
		void render() const;

		void pushRenderPass(const RenderPass &renderPass);
	};
}