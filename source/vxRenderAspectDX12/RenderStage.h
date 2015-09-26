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

	void buildCommands()
	{
		for (auto &it : m_renderPasses)
		{
			it->buildCommands();
		}
	}

	void submitCommands(Graphics::CommandQueue* queue)
	{
		for (auto &it : m_renderPasses)
		{
			it->submitCommands(queue);
		}
	}

	void submitAndExecuteCommands(Graphics::CommandQueue* queue)
	{
		submitCommands(queue);
		queue->execute();
	}
};