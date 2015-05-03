#pragma once

namespace Graphics
{
	class CapabilitySetting;
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
#include "BufferManager.h"
#include <vxLib/gl/ShaderManager.h>
#include <mutex>
#include <vector>
#include "RenderCommandFinalImage.h"
#include "RenderStage.h"
#include "CapabilityManager.h"

class VX_ALIGN(64) RenderAspect : public EventListener
{
	static const auto s_shadowMapResolution = 2048;

protected:
	struct ColdData
	{
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

		vx::gl::Texture m_shadowTexture;
		vx::gl::Texture m_ambientColorTexture;
		vx::gl::Texture m_ambientColorBlurTexture[2];
		// contains index into texture array sorted by texture handle

		Font m_font;
		Graphics::CapabilityManager m_capabilityManager;
	};

	Graphics::RenderStage m_renderStageCreateShadowMap;
	vx::uint2 m_resolution;
	SceneRenderer m_sceneRenderer;
	VoxelRenderer m_voxelRenderer;
	RenderCommand* m_pRenderPassFinalImage;
	std::mutex m_updateMutex;
	std::vector<RenderUpdateTask> m_tasks;
	RenderUpdateCameraData m_updateCameraData;

	vx::gl::Buffer m_cameraBuffer;
	
	vx::gl::Framebuffer m_shadowFB;
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
	BufferManager m_bufferManager;
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

	void createShadowMap(const vx::gl::VertexArray &vao, const vx::gl::Buffer &cmdBuffer, U32 count);

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

	void readFrame();
	void getFrameData(vx::float4a* dst);

	const vx::gl::ShaderManager& getShaderManager() const { return m_shaderManager; }
	void getProjectionMatrix(vx::mat4* m);
};