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

struct ID3D12Resource;
struct ID3D12CommandAllocator;
struct ID3D12CommandSignature;

class Scene;
class ResourceAspect;
struct ResourceView;
struct Light;
class RenderPass;
class GBufferRenderer;
class RenderPassVoxelize;

namespace vx
{
	class MessageManager;
}

#include <vxEngineLib/Graphics/RenderAspectInterface.h>
#include <vxEngineLib/Graphics/RenderLayer.h>
#include "CommandQueue.h"
#include "Device.h"
#include "RenderSettings.h"
#include "CopyManager.h"
#include "ResourceManager.h"
#include "Debug.h"
#include "Frustum.h"
#include "UploadManager.h"
#include "ShaderManager.h"
#include "MaterialManager.h"
#include <vxLib/Graphics/Camera.h>

class RenderAspect : public RenderAspectInterface
{
	static RenderSettings s_settings;

	d3d::CommandQueue m_graphicsCommandQueue;
	d3d::Device m_device;
	std::vector<Graphics::RenderLayer*> m_activeLayers;
	CopyManager m_copyManager;
	d3d::ResourceManager m_resourceManager;
	vx::Camera m_camera;
	d3d::Debug m_debug;
	Frustum m_frustum;
	UploadManager m_uploadManager;
	MaterialManager m_materialManager;
	vx::TaskManager* m_taskManager;
	ResourceAspectInterface* m_resourceAspect;
	vx::StackAllocator m_allocator;
	vx::MessageManager* m_msgManager;
	d3d::ShaderManager m_shaderManager;

	void getRequiredMemory(const vx::uint3 &dimSrgb, const vx::uint3 &dimRgb, u64* bufferHeapSize, u64* textureHeapSize, u64* rtDsHeapSize);

	bool createConstantBuffers();
	void uploadStaticCameraData();

	void updateCamera(const RenderUpdateCameraData &data);

	void createCbvCamera();
	void createSrvTransformPrev(u32 instanceCount);
	void createSrvTransform(u32 instanceCount);
	void createSrvMaterial(u32 instanceCount);
	void createSrvTextures(u32 srgbCount, u32 rgbCount);

	RenderAspectInitializeError initializeImpl(const RenderAspectDescription &desc) override;

public:
	RenderAspect();
	~RenderAspect();

	void shutdown(void* hwnd) override;

	bool initializeProfiler(Logfile* errorlog);

	void makeCurrent(bool b);

	void queueUpdate(RenderUpdateTaskType type, const u8* data, u32 dataSize) override;
	void queueUpdateCamera(const RenderUpdateCameraData &data) override;
	void update() override;

	void updateProfiler(f32 dt) override;

	void submitCommands() override;
	void endFrame() override;

	void handleMessage(const vx::Message &msg) override;

	void keyPressed(u16 key);

	void getProjectionMatrix(vx::mat4* m) const override;

	void getTotalVRam(u32* totalVram) const;
	void getTotalAvailableVRam(u32* totalAvailableVram) const;
	void getAvailableVRam(u32* availableVram) const;
};
