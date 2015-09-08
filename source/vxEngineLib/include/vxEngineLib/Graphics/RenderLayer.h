#pragma once

namespace vx
{
	class StackAllocator;
}

#include <vxLib/types.h>
#include <vxEngineLib/MessageListener.h>
#include <vxEngineLib/Graphics/CommandQueue.h>
#include <vxEngineLib/Graphics/RenderUpdateTask.h>

namespace Graphics
{
	class RenderLayer : public ::vx::MessageListener
	{
	public:
		virtual ~RenderLayer() {}

		virtual void createRenderPasses() = 0;

		virtual void getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs) = 0;

		virtual bool initialize(vx::StackAllocator* allocator) = 0;
		virtual void shudown() = 0;

		virtual void update() = 0;

		virtual void queueUpdate(const RenderUpdateTaskType type, const u8* data, u32 dataSize) = 0;

		virtual void submitCommandLists(Graphics::CommandQueue* queue) = 0;

		virtual u32 getCommandListCount() const = 0;
	};
}