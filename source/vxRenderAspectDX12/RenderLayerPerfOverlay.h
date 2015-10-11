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

namespace Graphics
{
	class Font;
}

namespace d3d
{
	class ResourceManager;
	class Device;
}

class RenderPass;
class ResourceAspectInterface;
class UploadManager;

#include <vxEngineLib/Graphics/RenderLayer.h>
#include "TextRenderer.h"
#include <vxLib/Container/sorted_vector.h>
#include "CommandAllocator.h"

class RenderLayerPerfOverlay : public Graphics::RenderLayer
{
	Graphics::TextRenderer m_textRenderer;
	const Graphics::Font* m_font;
	UploadManager* m_uploadManager;
	d3d::ResourceManager* m_resourceManager;
	d3d::Device* m_device;
	ResourceAspectInterface* m_resourceAspect;
	std::unique_ptr<d3d::CommandAllocator[]> m_allocators;
	vx::sorted_vector<vx::StringID, std::unique_ptr<RenderPass>> m_renderPasses;
	vx::uint2 m_resolution;

public:
	RenderLayerPerfOverlay(d3d::Device* device, ResourceAspectInterface* resourceAspectInterface, UploadManager* uploadManager, d3d::ResourceManager* resourceManager, const vx::uint2 &resolution);
	~RenderLayerPerfOverlay();

	void createRenderPasses() override;

	bool createData() override;

	void getRequiredMemory(u64* heapSizeBuffer, u32* bufferCount, u64* heapSizeTexture, u32* textureCount, u64* heapSizeRtDs, u32* rtDsCount) override;

	bool initialize(u32 frameCount, vx::StackAllocator* allocator, Logfile* errorLog) override;
	void shudown() override;

	void update() override;

	void queueUpdate(const RenderUpdateTaskType type, const u8* data, u32 dataSize) override;

	void buildCommandLists(u32 frameIndex) override;
	void submitCommandLists(Graphics::CommandQueue* queue) override;
	void buildAndSubmitCommandLists(u32 frameIndex, Graphics::CommandQueue* queue) override;

	u32 getCommandListCount() const override;

	void handleMessage(const vx::Message &evt) override;
};