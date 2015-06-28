#include <vxRenderAspect/dllExport.h>
#include <vxRenderAspect/RenderAspect.h>

RenderAspectInterface* createRenderAspect(const RenderAspectDescription &desc)
{
	auto result = (RenderAspect*)_aligned_malloc(sizeof(RenderAspect), __alignof(RenderAspect));
	new (result) RenderAspect{};

	if(!result->initialize(desc))
	{
		result->~RenderAspect();
		_aligned_free(result);
		result = nullptr;
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

Editor::RenderAspectInterface* createEditorRenderAspect(const RenderAspectDescription &desc)
{
	/*auto result = (RenderAspect*)_aligned_malloc(sizeof(RenderAspect), __alignof(RenderAspect));
	new (result) RenderAspect{};

	if (!result->initialize(desc))
	{
		result->~RenderAspect();
		_aligned_free(result);
		result = nullptr;
	}

	return result;*/

	return nullptr;
}

void destroyEditorRenderAspect(Editor::RenderAspectInterface *p)
{
	if (p != nullptr)
	{
		//auto ptr = (RenderAspect*)p;
		//ptr->~RenderAspect();
		//_aligned_free(ptr);
	}
}