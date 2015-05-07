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
#pragma once

namespace Graphics
{
	class Renderer;
	class ShadowRenderer;
}

class GpuProfiler;

#include "RenderAspectDescription.h"
#include "EventListener.h"
#include <vxLib\gl\RenderContext.h>
#include <vxLib\Graphics\Camera.h>
#include "SceneRenderer.h"
#include "VoxelRenderer.h"
#include "Font.h"
#include "RenderUpdateTask.h"
#include "gl/ObjectManager.h"
#include <vxLib/gl/ShaderManager.h>
#include <mutex>
#include <vector>
#include "RenderCommandFinalImage.h"
#include "RenderStage.h"
#include "RenderSettings.h"
#include "Graphics/CommandList.h"

class VX_ALIGN(64) RenderAspect : public EventListener
{
	static const auto s_shadowMapResolution = 2048;

protected:
	struct ColdData
	{
		RenderSettings m_settings;
		vx::gl::Texture m_gbufferDepthTexture;
		// albedoSlice : rgb8
		vx::gl::Texture m_gbufferAlbedoSlice;
		// normalSlice : rgb10a2
		vx::gl::Texture m_gbufferNormalSlice;
		// surface : rgba8
		vx::gl::Texture m_gbufferSurfaceSlice;
		// surface : rgbaf16
		vx::gl::Texture m_gbufferTangentSlice;
		vx::gl::Texture m_aabbTexture;
		vx::gl::Buffer m_screenshotBuffer;

		vx::gl::Texture m_ambientColorTexture;
		vx::gl::Texture m_ambientColorBlurTexture[2];
		// contains index into texture array sorted by texture handle

		Font m_font;
	//	std::vector<std::unique_ptr<Graphics::Renderer>> m_renderers;
	};

	Graphics::CommandList m_commandList;
	std::unique_ptr<Graphics::ShadowRenderer> m_shadowRenderer;
	vx::uint2 m_resolution;
	SceneRenderer m_sceneRenderer;
	VoxelRenderer m_voxelRenderer;
	RenderCommand* m_pRenderPassFinalImage;
	std::mutex m_updateMutex;
	std::vector<RenderUpdateTask> m_tasks;
	RenderUpdateCameraData m_updateCameraData;

	vx::gl::Buffer m_cameraBuffer;
	
	//vx::gl::Framebuffer m_shadowFB;
	vx::gl::Framebuffer m_gbufferFB;
	vx::gl::Framebuffer m_aabbFB;
	vx::gl::Framebuffer m_coneTraceFB;
	vx::gl::Framebuffer m_blurFB[2];
	vx::gl::VertexArray m_emptyVao;

	RenderCommandFinalImage m_renderpassFinalImageFullShading;
	RenderCommandFinalImage m_renderpassFinalImageAlbedo;
	RenderCommandFinalImage m_renderpassFinalImageNormals;
	vx::gl::ShaderManager m_shaderManager;
	vx::gl::RenderContext m_renderContext;
	vx::Camera m_camera;
	
	vx::StackAllocator m_allocator;

	Scene* m_pScene{nullptr};
	gl::ObjectManager m_objectManager;
	std::unique_ptr<ColdData> m_pColdData;

	bool createBuffers();
	void createUniformBuffers();

	void takeScreenshot();

	void writeMeshToVertexBuffer(const vx::StringID &meshSid, const vx::Mesh* pMesh, U32 *vertexOffsetGpu, U32 *indexOffsetGpu);

	void createTextures();
	void createFrameBuffers();

	bool initializeImpl(const std::string &dataDir, const vx::uint2 &windowResolution, bool debug, vx::StackAllocator *pAllocator);

	////////////// Event handling
	void handleFileEvent(const Event &evt);
	void handleIngameEvent(const Event &evt);
	//////////////

	void bindBuffers();

	void clearTextures();
	void clearBuffers();

	void voxelize(const vx::gl::VertexArray &vao, const vx::gl::Buffer &cmdBuffer, U32 count);
	void voxelDebug();

	void createGBuffer(const vx::gl::VertexArray &vao, const vx::gl::Buffer &cmdBuffer, U32 count);

	void coneTrace();
	void blurAmbientColor();

	void renderProfiler(GpuProfiler* gpuProfiler);

	void taskUpdateCamera();
	void taskTakeScreenshot();
	void taskLoadScene(void* p);
	void taskToggleRenderMode();
	void taskCreateActorGpuIndex(void* p);
	void taskUpdateDynamicTransforms(void* p);

	U16 addActorToBuffer(const vx::Transform &transform, const vx::StringID &mesh, const vx::StringID &material, const Scene* pScene);
	U16 getActorGpuIndex();

	void createRenderPassCreateShadowMaps();

public:
	RenderAspect();
	virtual ~RenderAspect();

	bool initialize(const std::string &dataDir, const RenderAspectDescription &desc);
	void shutdown(const HWND hwnd);

	bool initializeProfiler(GpuProfiler* gpuProfiler, vx::StackAllocator* allocator);

	void makeCurrent(bool b);

	void queueUpdateTask(const RenderUpdateTask &task);
	void queueUpdateCamera(const RenderUpdateCameraData &data);
	void update();

	void render(GpuProfiler* gpuProfiler);

	virtual void handleEvent(const Event &evt) override;

	void keyPressed(U16 key);

	const vx::gl::ShaderManager& getShaderManager() const { return m_shaderManager; }
	void getProjectionMatrix(vx::mat4* m);
};