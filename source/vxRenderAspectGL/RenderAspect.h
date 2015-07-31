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

namespace vx
{
	class MeshFile;
}

class GpuProfiler;

#include <vxEngineLib/RenderAspectInterface.h>
#include "RenderAspectDescription.h"
#include <vxEngineLib/EventListener.h>
#include <vxGL/RenderContext.h>
#include <vxLib\Graphics\Camera.h>
#include "Font.h"
#include "gl/ObjectManager.h"
#include <vxGL/ShaderManager.h>
#include <vector>
#include "RenderCommandFinalImage.h"
#include "Graphics/CommandList.h"
#include <vxEngineLib/DoubleBufferRaw.h>
#include "Graphics/Frame.h"
#include "opencl/context.h"
#include <vxEngineLib/mutex.h>
#include "Graphics/TextRenderer.h"
#include "Graphics/LightRenderer.h"
#include <vxGL/Framebuffer.h>
#include "Graphics/PStateProfiler.h"
#include "MaterialManager.h"
#include "MeshManager.h"

class VX_ALIGN(64) RenderAspect : public RenderAspectInterface
{
protected:
	struct ColdData;

	void* m_fence;
	Graphics::LightRenderer* m_lightRenderer;
	Graphics::Frame m_frame;
	Graphics::CommandList m_textCmdList;
	std::vector<std::unique_ptr<Graphics::Renderer>> m_renderer;
	vx::uint2 m_resolution;
	//SceneRenderer m_sceneRenderer;
	RenderCommand* m_pRenderPassFinalImage;
	vx::mutex m_updateMutex;
	std::vector<RenderUpdateTaskType> m_tasks;
	RenderUpdateCameraData m_updateCameraData;
	DoubleBufferRaw m_doubleBuffer;
	Graphics::ShadowRenderer* m_shadowRenderer;
	std::unique_ptr<Graphics::TextRenderer> m_textRenderer;
	std::unique_ptr<GpuProfiler> m_gpuProfiler;
	Graphics::PStateProfiler m_pstateProfiler;

	vx::gl::Buffer m_cameraBuffer;
	
	vx::gl::Framebuffer m_aabbFB;
	vx::gl::Framebuffer m_coneTraceFB;
	vx::gl::Framebuffer m_blurFB[2];

	RenderCommandFinalImage m_renderpassFinalImageFullShading;
	RenderCommandFinalImage m_renderpassFinalImageAlbedo;
	RenderCommandFinalImage m_renderpassFinalImageNormals;
	vx::gl::ShaderManager m_shaderManager;
	vx::gl::RenderContext m_renderContext;
	vx::Camera m_camera;
	vx::mat4 m_projectionMatrix;
	cl::Context m_context;
	FileAspectInterface* m_fileAspect;
	vx::EventManager* m_evtManager;
	
	vx::StackAllocator m_allocator;
	vx::StackAllocator m_scratchAllocator;
	MeshManager m_meshManager;
	MaterialManager m_materialManager;
	gl::ObjectManager m_objectManager;
	std::unique_ptr<ColdData> m_pColdData;

	virtual void createFrame();

	bool createBuffers();
	void createUniformBuffers(f32 znear, f32 zfar);

	void takeScreenshot();

	void writeMeshToVertexBuffer(const vx::StringID &meshSid, const vx::MeshFile* pMesh, u32 *vertexOffsetGpu, u32 *indexOffsetGpu);

	void createTextures();
	void createFrameBuffers();

	////////////// Event handling
	void handleFileEvent(const vx::Event &evt);
	//////////////

	void bindBuffers();

	void clearTextures();
	void clearBuffers();

	void voxelDebug();

	void createConeTracePixelList();
	void coneTrace();
	void blurAmbientColor();

	void renderProfiler();

	void processTasks();
	void taskUpdateCamera();
	void taskTakeScreenshot();
	void taskUpdateText(u8* p, u32* offset);
	void taskLoadScene(u8* p, u32* offset);
	void taskToggleRenderMode();
	void taskCreateActorGpuIndex(u8* p, u32* offset);
	void taskUpdateDynamicTransforms(u8* p, u32* offset);
	void taskAddStaticMeshInstance(u8* p, u32* offset);
	void taskAddDynamicMeshInstance(u8* p, u32* offset);

	u16 addActorToBuffer(const vx::StringID &actorSid, const vx::Transform &transform, const vx::StringID &mesh, const vx::StringID &material);
	u16 getActorGpuIndex();

	void createOpenCL();

public:
	RenderAspect();
	virtual ~RenderAspect();

	RenderAspectInitializeError initialize(const RenderAspectDescription &desc) override;
	void shutdown(void* hwnd) override;

	bool initializeProfiler() override;

	void makeCurrent(bool b) override;

	void queueUpdateTask(RenderUpdateTaskType type, const u8* data, u32 dataSize) override;
	void queueUpdateCamera(const RenderUpdateCameraData &data) override;
	void update() override;

	void updateProfiler(f32 dt) override;

	void submitCommands() override;
	void endFrame() override;

	virtual void handleEvent(const vx::Event &evt) override;

	void keyPressed(u16 key) override;

	void getProjectionMatrix(vx::mat4* m) override;

	void getTotalVRam(u32* totalVram) const override;
	void getTotalAvailableVRam(u32* totalAvailableVram) const override;
	void getAvailableVRam(u32* usedVram) const override;
};