#pragma once

namespace vx
{
	//struct Keyboard;
	//struct Mouse;
	//class Window;
	//class StackAllocator;
	//class Mesh;
	//struct Transform;
	//struct TransformGpu;
}

class Profiler2;
class ProfilerGraph;

#include "EventListener.h"
#include <vxLib\gl\RenderContext.h>
#include <vxLib\Graphics\Camera.h>
#include "SceneRenderer.h"
#include "VoxelRenderer.h"
#include "Font.h"
#include "RenderUpdateTask.h"
#include "NavMeshRenderer.h"
#include "BufferManager.h"
#include <vxLib/gl/ShaderManager.h>
#include <mutex>
#include <vector>

struct RenderAspectDesc
{
	const vx::Window* window;
	vx::StackAllocator* pAllocator;
	Profiler2 *pProfiler;
	ProfilerGraph* pGraph;
	vx::uint2 resolution;
	F32 fovRad; 
	F32 z_near;
	F32 z_far; 
	F32 targetMs;
	bool vsync; 
	bool debug;
};

class RenderAspect : public EventListener
{

	static const auto s_shadowMapResolution = 4096;

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
		vx::gl::Texture m_testTexture;
		vx::gl::Buffer m_screenshotBuffer;

		vx::gl::Texture m_shadowTexture;
		vx::gl::Texture m_ambientColorTexture;
		vx::gl::Texture m_ambientColorBlurTexture;

		vx::uint2 m_windowResolution;
		// contains index into texture array sorted by texture handle

		Font m_font;
	};

	SceneRenderer m_sceneRenderer;
	VoxelRenderer m_voxelRenderer;
	std::mutex m_updateMutex;
	std::vector<RenderUpdateTask> m_tasks;
	RenderUpdateCameraData m_updateCameraData;
	NavMeshRenderer m_navMeshRenderer;

	vx::gl::Buffer m_cameraBuffer;
	
	vx::gl::Framebuffer m_shadowFB;
	vx::gl::Framebuffer m_gbufferFB;
	vx::gl::Framebuffer m_aabbFB;
	vx::gl::Framebuffer m_coneTraceFB;
	vx::gl::Framebuffer m_blurFB;
	vx::gl::VertexArray m_emptyVao;

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

	void writeMeshToVertexBuffer(const vx::StringID64 &meshSid, const vx::Mesh* pMesh, U32 *vertexOffsetGpu, U32 *indexOffsetGpu);

	void createTextures();
	void createFrameBuffers();

	bool initializeImpl(const std::string &dataDir, const vx::uint2 &windowResolution, bool debug, F32 targetMs,
		vx::StackAllocator *pAllocator, Profiler2 *pProfiler, ProfilerGraph* pGraph);

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

	void renderFinalImage();
	void renderProfiler(Profiler2* pProfiler, ProfilerGraph* pGraph);
	void renderNavGraph();

	void taskUpdateCamera();
	void taskTakeScreenshot();
	void taskLoadScene(void* p);

	// returns index into transform buffer
	U16 getActorGpuIndex();
	U16 addActorToBuffer(const vx::Transform &transform, const vx::StringID64 &mesh, const vx::StringID64 &material, const Scene* pScene);

public:
	RenderAspect();
	virtual ~RenderAspect();

	bool initialize(const std::string &dataDir, const RenderAspectDesc &desc);
	void shutdown(const HWND hwnd);

	void makeCurrent(bool b);

	void queueUpdateTask(const RenderUpdateTask &task);
	void queueUpdateCamera(const RenderUpdateCameraData &data);
	void update();

	void render();

	virtual void handleEvent(const Event &evt) override;

	const vx::gl::ShaderManager& getShaderManager() const { return m_shaderManager; }
	void getProjectionMatrix(vx::mat4* m);
};