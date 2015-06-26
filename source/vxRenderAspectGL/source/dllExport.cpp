#include <vxRenderAspect/dllExport.h>
#include <vxRenderAspect/RenderAspect.h>

RenderAspectInterface* createRenderAspect(const std::string &dataDir, const RenderAspectDescription &desc, const EngineConfig* settings, FileAspect* fileAspect, vx::EventManager* evtManager)
{
	auto result = (RenderAspect*)_aligned_malloc(sizeof(RenderAspect), __alignof(RenderAspect));
	new (result) RenderAspect{};

	if(!result->initialize(dataDir, desc, settings, fileAspect, evtManager))
	{
		result->~RenderAspect();
		_aligned_free(result);
		result = nullptr;
	}

	return result;
}