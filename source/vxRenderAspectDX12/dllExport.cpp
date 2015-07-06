#include "dllExport.h"
#include "RenderAspect.h"

RenderAspectInterface* createRenderAspect(const RenderAspectDescription &desc, RenderAspectInitializeError* error)
{
	auto result = (RenderAspect*)_aligned_malloc(sizeof(RenderAspect), __alignof(RenderAspect));
	new (result) RenderAspect{};

	auto initError = result->initialize(desc);
	if (initError != RenderAspectInitializeError::OK)
	{
		result->~RenderAspect();
		_aligned_free(result);
		result = nullptr;
	}

	if (error)
	{
		*error = initError;
	}

	return result;
}

void destroyRenderAspect(RenderAspectInterface *p)
{
	if (p != nullptr)
	{
		auto ptr = (RenderAspect*)p;
		ptr->~RenderAspect();
		_aligned_free(ptr);
	}
}

Editor::RenderAspectInterface* createEditorRenderAspect(const RenderAspectDescription &desc, RenderAspectInitializeError* error)
{
	return nullptr;
}

void destroyEditorRenderAspect(Editor::RenderAspectInterface *p)
{
	if (p != nullptr)
	{
	}
}