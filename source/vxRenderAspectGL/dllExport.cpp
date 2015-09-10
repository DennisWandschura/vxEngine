#include "dllExport.h"
#include "RenderAspect.h"
#include "EditorRenderAspect.h"
#include <vxEngineLib/debugPrint.h>

RenderAspectInterface* createRenderAspect()
{
	auto result = (RenderAspect*)_aligned_malloc(sizeof(RenderAspect), __alignof(RenderAspect));
	new (result) RenderAspect{};

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

Editor::RenderAspectInterface* createEditorRenderAspect()
{
	auto result = (Editor::RenderAspect*)_aligned_malloc(sizeof(Editor::RenderAspect), __alignof(Editor::RenderAspect));
	new (result)Editor::RenderAspect{};

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