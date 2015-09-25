#pragma once

#include "RenderPass.h"
#include <vector>

class RenderStage
{
	std::vector<RenderPass*> m_renderPasses;

public:
	RenderStage() :m_renderPasses() {}
	~RenderStage() {}

	void pushRenderPass(RenderPass* rp)
	{
		m_renderPasses.push_back(rp);
	}

	void execute(Graphics::CommandQueue* queue)
	{
		for (auto &it : m_renderPasses)
		{
			it->submitCommands(queue);
		}
		queue->execute();
	}
};