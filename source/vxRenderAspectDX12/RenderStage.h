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

	void buildCommands(d3d::CommandAllocator* currentAllocator, u32 frameIndex)
	{
		for (auto &it : m_renderPasses)
		{
			it->buildCommands(currentAllocator, frameIndex);
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