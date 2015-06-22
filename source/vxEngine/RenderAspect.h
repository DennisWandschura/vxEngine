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
	struct RendererSettings;
	class Renderer;
	class ShadowRenderer;
}

namespace Editor
{
	class Scene;
}

struct EngineConfig;
class GpuProfiler;

#include "RenderAspectDescription.h"
#include <vxEngineLib/EventListener.h>
#include <vxLib\gl\RenderContext.h>
#include <vxLib\Graphics\Camera.h>
#include "SceneRenderer.h"
#include "Font.h"
#include "RenderUpdateTask.h"
#include "gl/ObjectManager.h"
#include <vxLib/gl/ShaderManager.h>
#include <mutex>
#include <vector>
#include "RenderCommandFinalImage.h"
#include "Graphics/CommandList.h"
#include "DoubleBufferRaw.h"
#include "Graphics/Frame.h"
#include "opencl/context.h"

class VX_ALIGN(64) RenderAspect : public vx::EventListener
{
protected:
	struct ColdData;

	Graphics::Frame m_frame;
	std::vector<std::unique_ptr<Graphics::Renderer>> m_renderer;
	vx::uint2 m_resolution;
	SceneRenderer m_sceneRenderer;
	RenderCommand* m_pRenderPassFinalImage;
	std::mutex m_updateMutex;
	std::vector<RenderUpdateTask> m_tasks;
	RenderUpdateCameraData m_updateCameraData;
	DoubleBufferRaw m_doubleBuffer;
	Graphics::ShadowRenderer* m_shadowRenderer;

	vx::gl::Buffer m_cameraBuffer;
	
	vx::gl::Framebuffer m_gbufferFB;
	vx::gl::Framebuffer m_aabbFB;
	vx::gl::Framebuffer m_coneTraceFB;
	vx::gl::Framebuffer m_blurFB[2];

	RenderCommandFinalImage m_renderpassFinalImageFullShading;
	RenderCommandFinalImage m_renderpassFinalImageAlbedo;
	RenderCommandFinalImage m_renderpassFinalImageNormals;
	vx::gl::ShaderManager m_shaderManager;
	vx::gl::RenderContext m_renderContext;
	vx::Camera m_camera;
	cl::Context m_context;
	
	vx::StackAllocator m_allocator;

	gl::ObjectManager m_objectManager;
	std::unique_ptr<ColdData> m_pColdData;

	virtual void createFrame();

	bool createBuffers();
	void createUniformBuffers(f32 znear, f32 zfar);

	void takeScreenshot();

	void writeMeshToVertexBuffer(const vx::StringID &meshSid, const vx::MeshFile* pMesh, u32 *vertexOffsetGpu, u32 *indexOffsetGpu);

	void createTextures();
	void createFrameBuffers();

	bool initializeCommon(const vx::gl::ContextDescription &desc, const Graphics::RendererSettings &options);
	bool initializeImpl(const std::string &dataDir, const EngineConfig* settings, vx::StackAllocator *pAllocator);

	////////////// Event handling
	void handleFileEvent(const vx::Event &evt);
	void handleIngameEvent(const vx::Event &evt);
	//////////////

	void bindBuffers();

	void clearTextures();
	void clearBuffers();

	void voxelize(const vx::gl::VertexArray &vao, const vx::gl::Buffer &cmdBuffer, const vx::gl::Buffer &paramBuffer);
	void voxelDebug();

	void createConeTracePixelList();
	void coneTrace();
	void blurAmbientColor();

	void renderProfiler(GpuProfiler* gpuProfiler);

	void taskUpdateCamera();
	void taskTakeScreenshot();
	void taskLoadScene(u8* p, u32* offset);
	void taskToggleRenderMode();
	void taskCreateActorGpuIndex(u8* p, u32* offset);
	void taskUpdateDynamicTransforms(u8* p, u32* offset);

	u16 addActorToBuffer(const vx::Transform &transform, const vx::StringID &mesh, const vx::StringID &material);
	u16 getActorGpuIndex();

	void createColdData();
	void provideRenderData(const Graphics::RendererSettings* settings);

	void createOpenCL();

public:
	RenderAspect();
	virtual ~RenderAspect();

	bool initialize(const std::string &dataDir, const RenderAspectDescription &desc, const EngineConfig* settings);
	void shutdown(const HWND hwnd);

	bool initializeProfiler(GpuProfiler* gpuProfiler, vx::StackAllocator* allocator);

	void makeCurrent(bool b);

	void queueUpdateTask(const RenderUpdateTask &task);
	void queueUpdateTask(const RenderUpdateTask &task, const u8* data, u32 dataSize);
	void queueUpdateCamera(const RenderUpdateCameraData &data);
	void update();

	void render(GpuProfiler* gpuProfiler);

	virtual void handleEvent(const vx::Event &evt) override;

	void keyPressed(u16 key);

	const vx::gl::ShaderManager& getShaderManager() const { return m_shaderManager; }
	void getProjectionMatrix(vx::mat4* m);
	const Font& getProfilerFont() const;
};