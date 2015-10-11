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

#include "RenderLayerPerfOverlay.h"
#include <vxEngineLib/ResourceAspectInterface.h>
#include <vxEngineLib/Message.h>
#include <vxEngineLib/MessageTypes.h>
#include <vxEngineLib/FileMessage.h>
#include <vxEngineLib/Graphics/Texture.h>
#include <vxEngineLib/Graphics/Font.h>
#include "UploadManager.h"
#include <d3d12.h>
#include "ResourceManager.h"
#include "RenderPassText.h"
#include "Device.h"
#include <vxEngineLib/Logfile.h>
#include "ResourceDesc.h"
#include <vxEngineLib/CpuTimer.h>

namespace RenderLayerPerfOverlayCpp
{
	const u32 g_maxCharacters = 2048;
}

RenderLayerPerfOverlay::RenderLayerPerfOverlay(d3d::Device* device, ResourceAspectInterface* resourceAspectInterface, UploadManager* uploadManager, d3d::ResourceManager* resourceManager, const vx::uint2 &resolution)
	:Graphics::RenderLayer(),
	m_textRenderer(),
	m_font(nullptr),
	m_uploadManager(uploadManager),
	m_resourceManager(resourceManager),
	m_device(device),
	m_resourceAspect(resourceAspectInterface),
	m_resolution(resolution)
{

}

RenderLayerPerfOverlay::~RenderLayerPerfOverlay()
{

}

void RenderLayerPerfOverlay::createRenderPasses()
{
	m_renderPasses.insert(vx::make_sid("RenderPassText"), std::unique_ptr<RenderPass>(new RenderPassText(m_device)));
}

void RenderLayerPerfOverlay::getRequiredMemory(u64* heapSizeBuffer, u32* bufferCount, u64* heapSizeTexture, u32* textureCount, u64* heapSizeRtDs, u32* rtDsCount)
{
	auto device = m_device->getDevice();
	m_textRenderer.getRequiredMemory(heapSizeBuffer, bufferCount, heapSizeTexture, textureCount, device, RenderLayerPerfOverlayCpp::g_maxCharacters);

	for (auto &it : m_renderPasses)
	{
		it->getRequiredMemory(heapSizeBuffer, bufferCount, heapSizeTexture, textureCount, heapSizeRtDs, rtDsCount, device);
	}

	d3d::ResourceDesc resDesc = d3d::ResourceDesc::getDescTexture2D(m_resolution, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
	resDesc.DepthOrArraySize = 2;
	auto allocInfo = device->GetResourceAllocationInfo(1, 1, &resDesc);
	*heapSizeRtDs += allocInfo.SizeInBytes;
	*rtDsCount += 1;
}

bool RenderLayerPerfOverlay::createData()
{
	auto device = m_device->getDevice();
	if (!m_textRenderer.createData(device, m_resourceManager, m_uploadManager, RenderLayerPerfOverlayCpp::g_maxCharacters))
	{
		return false;
	}

	for (auto &it : m_renderPasses)
	{
		if (!it->createData(device))
			return false;
	}

	d3d::ResourceDesc resDesc = d3d::ResourceDesc::getDescTexture2D(m_resolution, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
	resDesc.DepthOrArraySize = 2;
	auto allocInfo = device->GetResourceAllocationInfo(1, 1, &resDesc);

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = resDesc.Format;

	CreateResourceDesc desc;
	desc.clearValue = &clearValue;
	desc.resDesc = &resDesc;
	desc.size = allocInfo.SizeInBytes;
	desc.state = D3D12_RESOURCE_STATE_RENDER_TARGET;

	auto ptr = m_resourceManager->createTextureRtDs(L"layerOverlayTexture", desc);

	return (ptr != nullptr);
}

bool RenderLayerPerfOverlay::initialize(u32 frameCount, vx::StackAllocator* allocator, Logfile* errorLog)
{
	auto device = m_device->getDevice();
	auto renderPassText = m_renderPasses.find(vx::make_sid("RenderPassText"));

	m_allocators = std::make_unique<d3d::CommandAllocator[]>(frameCount);
	for (u32 i = 0; i < frameCount; ++i)
	{
		if (!m_allocators[i].create(D3D12_COMMAND_LIST_TYPE_DIRECT, device))
			return false;
	}

	Graphics::TextRendererDesc desc;
	desc.allocator = allocator;
	desc.device = device;
	desc.resourceManager = m_resourceManager;
	desc.uploadManager = m_uploadManager;
	desc.maxCharacters = RenderLayerPerfOverlayCpp::g_maxCharacters;
	desc.m_renderPassText = (RenderPassText*)renderPassText->get();
	if (!m_textRenderer.initialize(allocator, &desc))
	{
		errorLog->append("error initializing text renderer\n");
		return false;
	}

	auto fontFileEntry = vx::FileEntry("verdana.font", vx::FileType::Font);
	vx::Variant arg;
	arg.u64 = fontFileEntry.getSid().value;
	m_resourceAspect->requestLoadFile(fontFileEntry, arg);


	for (auto &it : m_renderPasses)
	{
		it->initialize(device, m_allocators.get(), frameCount);
	}

	return true;
}

void RenderLayerPerfOverlay::shudown()
{

}

void RenderLayerPerfOverlay::update()
{
	static CpuTimer timer;

	auto elapsedTime = timer.getTimeSeconds();
	bool upload = (elapsedTime >= 0.25f);

	m_textRenderer.update(m_uploadManager, m_resourceManager, upload);

	if (upload)
	{
		timer.reset();
	}
}

void RenderLayerPerfOverlay::queueUpdate(const RenderUpdateTaskType type, const u8* data, u32 dataSize)
{
	switch (type)
	{
	case RenderUpdateTaskType::UpdateText:
	{
		auto updateData = (RenderUpdateTextData*)data;
		m_textRenderer.pushEntry(updateData->text, updateData->strSize, updateData->position, updateData->color);
	}break;
	default:
		break;
	}
}

void RenderLayerPerfOverlay::buildCommandLists(u32 frameIndex)
{
	auto idx = m_device->getCurrentBackBufferIndex();
	m_allocators[frameIndex]->Reset();
	for (auto &it : m_renderPasses)
	{
		it->buildCommands(&m_allocators[frameIndex], frameIndex);
	}
}

void RenderLayerPerfOverlay::submitCommandLists(Graphics::CommandQueue* queue)
{
	for (auto &it : m_renderPasses)
	{
		it->submitCommands(queue);
	}
	queue->execute();
}

void RenderLayerPerfOverlay::buildAndSubmitCommandLists(u32 frameIndex, Graphics::CommandQueue* queue)
{
	m_allocators[frameIndex]->Reset();
	for (auto &it : m_renderPasses)
	{
		it->buildCommands(&m_allocators[frameIndex], frameIndex);
		it->submitCommands(queue);
	}
	queue->execute();
}

u32 RenderLayerPerfOverlay::getCommandListCount() const
{
	return 0;
}

void RenderLayerPerfOverlay::handleMessage(const vx::Message &msg)
{
	auto type = msg.type;
	if (type == vx::MessageType::File)
	{
		vx::FileMessage msgType = (vx::FileMessage)msg.code;
		if (msgType == vx::FileMessage::Font_Loaded)
		{
			auto fontFileEntry = vx::FileEntry("verdana.font", vx::FileType::Font);

			if (msg.arg1.u64 == fontFileEntry.getSid().value)
			{
				auto fontFile = m_resourceAspect->getFontFile(fontFileEntry.getSid());

				auto fontTexture = m_resourceManager->getTexture(L"fontTexture");

				m_font = fontFile;
				m_textRenderer.setFont(fontFile);

				auto texture = fontFile->getTexture();
				auto format = texture->getFormat();
				auto &face = texture->getFace(0);
				auto dim = face.getDimension();

				UploadTaskTextureDesc desc;
				desc.data = face.getPixels();
				desc.dataSize = face.getSize();
				desc.dim = vx::uint2(dim.x, dim.y);
				desc.dst = fontTexture->get();
				desc.format = Graphics::textureFormatToDxgiFormat(format);
				desc.rowPitch = texture->getFaceRowPitch(0);
				desc.slice = 0;
				desc.state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
				m_uploadManager->pushUploadTexture(desc);
			}
		}
	}
}