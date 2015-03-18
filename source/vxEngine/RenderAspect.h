#pragma once

namespace vx
{
	struct Keyboard;
	struct Mouse;
	class Window;
	class StackAllocator;
	class Mesh;
	struct Transform;
	struct TransformGpu;
}

class Material;
class MeshInstance;
class FileEntry;
class FileAspect;
class Scene;
struct LoadFileReturnType;
struct LightTransformBlock;
struct Light;
class Profiler2;
class ProfilerGraph;
typedef struct __GLsync *GLsync;
struct VertexPNTUV;
class NavMesh;

#include "EventListener.h"
#include <vxLib\gl\RenderContext.h>
#include <vxLib\gl\StateManager.h>
#include <vxLib\Graphics\Camera.h>
#include "RenderStage.h"
#include "Font.h"
#include "ShaderManager.h"
#include <atomic>
#include <mutex>
#include "LoadFileCallback.h"
#include "StackAllocator.h"
#include "Squirrel.h"
#include "BVH.h"

inline vx::int2 __vectorcall packQRotation(const __m128 qRotation)
{
	const __m128 maxV = { INT16_MAX, INT16_MAX, INT16_MAX, INT16_MAX };

	// -1, 1 -> 0, 1
	//auto qq = _mm_fmadd_ps(qRotation, DirectX::g_XMOneHalf, DirectX::g_XMOneHalf);
	// 0, 1 -> 0, g_INT16_MAX
	//qq = _mm_mul_ps(qq, maxV);

	auto qq = _mm_mul_ps(qRotation, maxV);

	vx::int2 packedQRotation;
	packedQRotation.x = (I32)qq.m128_f32[0] | (I32)qq.m128_f32[1] << 16;
	packedQRotation.y = (I32)qq.m128_f32[2] | (I32)qq.m128_f32[3] << 16;

	return packedQRotation;
};

struct MeshEntry
{
	U32 firstIndex;
	U32 indexCount;
};

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
	static const auto s_voxelDimension = 128u;

	static const auto s_maxNavMeshVertices = 100u;
	static const auto s_maxNavMeshIndices = 100u;

	static const auto s_maxVerticesStatic = 500000u;
	static const auto s_maxVerticesDynamic = 200000u;
	static const auto s_maxVerticesTotal = s_maxVerticesStatic + s_maxVerticesDynamic;

	static const auto s_maxIndicesStatic = 40000u;
	static const auto s_maxIndicesDynamic = 20000u;
	static const auto s_maxIndicesTotal = s_maxIndicesStatic + s_maxIndicesDynamic;

	static const auto s_maxMaterials = 100u;
	static const auto s_meshMaxInstancesStatic = 100u;
	static const auto s_meshMaxInstancesDynamic = 50u;
	static const auto s_meshMaxInstances = s_meshMaxInstancesStatic + s_meshMaxInstancesDynamic;

protected:
	typedef  struct 
	{
		U32  count;
		U32  instanceCount;
		U32  first;
		U32  baseInstance;
	} DrawArraysIndirectCommand;

	struct ColdData
	{
		vx::gl::Texture m_gbufferDepthTexture;
		vx::gl::Texture m_gbufferDepthTextureSmall;
		// albedoSlice : rgb8
		vx::gl::Texture m_gbufferAlbedoSlice;
		// normalSlice : rgb10a2
		vx::gl::Texture m_gbufferNormalSlice;
		vx::gl::Texture m_gbufferNormalSliceSmall;
		// surface : rgba8
		vx::gl::Texture m_gbufferSurfaceSlice;
		vx::gl::Buffer m_screenshotBuffer;
		TextureManager m_textureManager;
		vx::gl::Buffer m_meshVbo;
		vx::gl::Buffer m_meshIdVbo;
		vx::gl::Buffer m_meshIbo;
		vx::gl::Buffer m_navmeshVbo;
		vx::gl::Buffer m_navmeshIbo;
		vx::gl::Buffer m_navNodesVbo;
		vx::gl::Buffer m_navNodesIbo;
		vx::gl::Buffer m_navConnectionsIbo;
		U32 m_meshVertexCount{ 0 };
		U32 m_meshIndexCount{ 0 };
		vx::uint2 m_windowResolution;
		// contains index into texture array sorted by texture handle
		vx::sorted_vector<U64, U32> m_texturesGPU;
		Font m_font;
	};

	// 
	vx::gl::Buffer m_cameraBuffer;
	vx::gl::Buffer m_cameraBufferStatic;
	vx::gl::Buffer m_lightDataBlock;
	vx::gl::Buffer m_gbufferBlock;
	vx::gl::Buffer m_voxelBlock;
	//
	vx::gl::Buffer m_lightTexBlock;
	vx::gl::Buffer m_transformBlock;
	vx::gl::Buffer m_materialBlock;
	//
	vx::gl::Buffer m_meshVertexBlock;
	vx::gl::Buffer m_bvhBlock;
	//
	vx::gl::Buffer m_textureBlock;
	// contains compressed rays
	vx::gl::Buffer m_rayList;
	vx::gl::Buffer m_rayLinks;
	vx::gl::Buffer m_rayLinkChunks;
	vx::gl::Buffer m_rayAtomicCounter;
	vx::gl::Texture m_rayTraceShadowTexture;
	vx::gl::Texture m_rayTraceShadowTextureSmall;
	vx::gl::Texture m_voxelTexture;
	// each cell contains amount of rays that need to be checked against
	vx::gl::Texture m_voxelRayCountTexture;
	
	vx::gl::Framebuffer m_gbufferFB;
	vx::gl::Framebuffer m_gbufferFBSmall;
	// batch without instancing
	vx::gl::DrawIndirectBuffer m_commandBlock;
	U32 m_numTiles{ 0 };
	// total count of mesh instances, no instancing
	U32 m_meshInstancesCountTotal{ 0 };
	U32 m_numPointLights{ 0 };
	vx::gl::VertexArray m_emptyVao;
	vx::gl::StateManager m_stateManager;
	vx::gl::VertexArray m_meshVao;
	vx::gl::VertexArray m_navmeshVao;
	U32 m_navmeshIndexCount{ 0 };
	vx::gl::VertexArray m_navNodesVao;
	U32 m_navNodesCount{ 0 };
	vx::gl::VertexArray m_navConnectionsVao;
	U32 m_navConnectionCount{ 0 };
	ShaderManager m_shaderManager;
	vx::gl::RenderContext m_renderContext;
	vx::Camera m_camera;
	vx::StackAllocator m_scratchAllocator;
	vx::StackAllocator m_allocator;
	U32 m_meshInstanceCountDynamic{ 0 };
	U32 m_meshInstanceCountStatic{ 0 };
	U32 m_meshVertexCountDynamic{0};
	U32 m_meshIndexCountDynamic{ 0 };
	U32 m_materialCount{ 0 };
	vx::sorted_vector<vx::StringID64, MeshEntry> m_meshEntries;
	vx::sorted_vector<const Material*, U32> m_materialIndices;
	//BVH m_bvh;
	Scene* m_pScene{nullptr};
	FileAspect &m_fileAspect;
	std::unique_ptr<ColdData> m_pColdData;

	/*
	tries to load a texture to gpu and return a reference to it
	on failure the reference is invalid
	*/
	TextureRef loadTextureFile(const TextureFile &file, U8 srgb);
	bool initializeBuffers();
	void initializeUniformBuffers();
	void createVoxelBuffer();

	void takeScreenshot();
	void writeScreenshot(const vx::uint2 &resolution, vx::float4 *pData);

	void writeTransform(const vx::Transform &t, U32 elementId);
	void writeMeshInstanceIdBuffer(U32 elementId, U32 materialIndex);
	void writeMeshInstanceToCommandBuffer(MeshEntry meshEntry, U32 index, U32 elementId);
	void createMaterial(Material* pMaterial);
	void writeMaterialToBuffer(const Material *pMaterial, U32 offset);
	void writeMeshToVertexBuffer(const vx::StringID64 &meshSid, const vx::Mesh* pMesh, U32 *vertexOffsetGpu, U32 *indexOffsetGpu);
	void writeMeshToBuffer(const vx::StringID64 &meshSid, const vx::Mesh* pMesh, VertexPNTUV* pVertices, U32* pIndices, U32* vertexOffset, U32* indexOffset, U32 *vertexOffsetGpu, U32 *indexOffsetGpu);
	void writeMeshesToVertexBuffer(const vx::StringID64* meshSid, const vx::Mesh** pMesh, U32 count, U32 *vertexOffsetGpu, U32 *indexOffsetGpu);
	void updateMeshBuffer(const vx::sorted_vector<vx::StringID64, const vx::Mesh*> &meshes);
	void updateBuffers(const MeshInstance *pInstances, U32 instanceCount, const vx::sorted_vector<const Material*, U32> &materialIndices, const vx::sorted_vector<vx::StringID64, MeshEntry> &meshEntries);

	void getShadowTransform(const Light &light, vx::mat4 *projMatrix, vx::mat4 *pvMatrices);
	void updateLightBuffer(const Light *pLights, U32 numLights);

	void createTextures();
	void createFrameBuffers();

	bool initializeImpl(const std::string &dataDir, const vx::uint2 &windowResolution, bool debug, F32 targetMs,
		vx::StackAllocator *pAllocator, Profiler2 *pProfiler, ProfilerGraph* pGraph);

	void loadCurrentScene(const Scene* pScene);

	////////////// Event handling
	void handleFileEvent(const Event &evt);
	void handleIngameEvent(const Event &evt);
	//////////////

	void updateNavMeshBuffer(const NavMesh &navMesh);

public:
	RenderAspect(Logfile &logfile, FileAspect &fileAspect);
	virtual ~RenderAspect();

	bool initialize(const std::string &dataDir, const RenderAspectDesc &desc);
	void shutdown(const HWND hwnd);

	void update();

	void updateTransform(U16 index, const vx::TransformGpu &transform);
	void updateTransforms(const vx::TransformGpu* transforms, U32 offsetCount, U32 count);

	void render(Profiler2* pProfiler, ProfilerGraph* pGraph);

	// returns index into transform buffer
	U16 getActorGpuIndex();
	U16 addActorToBuffer(const vx::Transform &transform, const vx::StringID64 &mesh, const vx::StringID64 &material, const Scene* pScene);

	void keyPressed(U16 key);
	void keyReleased(U16 key);

	virtual void handleEvent(const Event &evt) override;

	vx::Camera& getCamera(){ return m_camera; }
	const ShaderManager& getShaderManager() const { return m_shaderManager; }
	void getProjectionMatrix(vx::mat4* m);
};