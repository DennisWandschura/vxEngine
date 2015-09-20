#pragma once

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

#include <vxEngineLib/Graphics/RenderLayer.h>
#include "TextRenderer.h"
#include <vxEngineLib/Graphics/Font.h>

class RenderLayerPerfOverlay : public Graphics::RenderLayer
{
	Graphics::TextRenderer m_textRenderer;
	Graphics::Font m_font;
	ID3D12Device* m_device;

public:
	explicit RenderLayerPerfOverlay(ID3D12Device* device);
	~RenderLayerPerfOverlay();

	void createRenderPasses() override;

	void getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs) override;

	bool initialize(vx::StackAllocator* allocator) override;
	void shudown() override;

	void update() override;

	void queueUpdate(const RenderUpdateTaskType type, const u8* data, u32 dataSize) override;

	void submitCommandLists(Graphics::CommandQueue* queue) override;

	u32 getCommandListCount() const override;
};