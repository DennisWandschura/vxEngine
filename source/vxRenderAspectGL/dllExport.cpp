#include "dllExport.h"
#include "RenderAspect.h"
#include "EditorRenderAspect.h"
#include <vxEngineLib/debugPrint.h>

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
	auto result = (Editor::RenderAspect*)_aligned_malloc(sizeof(Editor::RenderAspect), __alignof(Editor::RenderAspect));
	new (result)Editor::RenderAspect{};

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

template<typename T>
void destroy(T* ptr)
{
	ptr->~T();
}

void destroyEditorRenderAspect(Editor::RenderAspectInterface *p)
{
	if (p != nullptr)
	{
		auto ptr = (Editor::RenderAspect*)p;
		destroy(ptr);
		_aligned_free(ptr);
	}
}