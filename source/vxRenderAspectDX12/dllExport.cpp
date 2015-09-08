/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "dllExport.h"
#include "RenderAspect.h"

RenderAspectInterface* createRenderAspect(const RenderAspectDescription &desc, AbortSignalHandlerFun signalHandler, RenderAspectInitializeError* error)
{
	const auto sz = sizeof(RenderAspect);
	const auto align = __alignof(RenderAspect);

	auto result = (RenderAspect*)_aligned_malloc(sz, align);
	new (result) RenderAspect{};

	auto initError = result->initialize(desc, signalHandler);
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

Editor::RenderAspectInterface* createEditorRenderAspect(const RenderAspectDescription &desc, AbortSignalHandlerFun signalHandler, RenderAspectInitializeError* error)
{
	return nullptr;
}

void destroyEditorRenderAspect(Editor::RenderAspectInterface *p)
{
	if (p != nullptr)
	{
	}
}