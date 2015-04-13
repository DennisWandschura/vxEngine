#pragma once

class Scene;
class BufferManager;
class NavMeshRenderer;
class Material;
class MeshInstance;
struct VertexPNTUV;

namespace vx
{
	namespace gl
	{
		class ShaderManager;
	}

	struct Transform;
	struct StringID64;
	class Mesh;
}

#include <vxLib/gl/Buffer.h>
#include <vxLib/gl/VertexArray.h>
#include "TextureManager.h"
#include "LightBufferManager.h"
#include <vxLib/Allocator/StackAllocator.h>

struct MeshEntry
{
	U32 firstIndex;
	U32 indexCount;
};

class SceneRenderer
{
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

	struct ColdData
	{
		vx::gl::Buffer m_transformBlock;
		vx::gl::Buffer m_materialBlock;

		vx::gl::Buffer m_meshVbo;
		vx::gl::Buffer m_meshIdVbo;
		vx::gl::Buffer m_meshIbo;
		U32 m_meshVertexCount{ 0 };
		U32 m_meshIndexCount{ 0 };
		U32 m_materialCount{ 0 };

		LightBufferManager m_lightBufferManager;
		U32 m_meshInstanceCountDynamic{ 0 };
		U32 m_meshInstanceCountStatic{ 0 };
		U32 m_meshVertexCountDynamic{ 0 };
		U32 m_meshIndexCountDynamic{ 0 };
		vx::StackAllocator m_scratchAllocator;
		TextureManager m_textureManager;
		vx::sorted_vector<const Material*, U32> m_materialIndices;
		vx::sorted_vector<vx::StringID64, MeshEntry> m_meshEntries;
		vx::sorted_vector<U64, U32> m_texturesGPU;
	};

	vx::gl::Buffer m_commandBlock;
	vx::gl::VertexArray m_meshVao;
	U32 m_meshInstancesCountTotal{ 0 };
	std::unique_ptr<ColdData> m_coldData;

	void createTextures(BufferManager* pBufferManager);
	void createMeshDrawIdVbo();
	void createMeshIbo();
	void bindMeshDrawIdVboToVao(vx::gl::VertexArray* vao);
	void createMeshBuffers();

	void createMeshCmdBuffer();
	void createMeshTransformBuffer();
	void createMeshMaterialBuffer();

	void createMaterial(Material* pMaterial);

	void updateTransform(const vx::Transform &t, U32 elementId);

	void writeMaterialToBuffer(const Material *pMaterial, U32 offset);
	void writeMeshToBuffer(const vx::StringID64 &meshSid, const vx::Mesh* pMesh, VertexPNTUV* pVertices, U32* pIndices, U32* vertexOffset, U32* indexOffset, U32 *vertexOffsetGpu, U32 *indexOffsetGpu);
	void writeMeshesToVertexBuffer(const vx::StringID64* meshSid, const vx::Mesh** pMesh, U32 count, U32 *vertexOffsetGpu, U32 *indexOffsetGpu);
	void writeMeshInstanceIdBuffer(U32 elementId, U32 materialIndex);
	void writeMeshInstanceToCommandBuffer(MeshEntry meshEntry, U32 index, U32 elementId);

	void updateMeshBuffer(const vx::sorted_vector<vx::StringID64, const vx::Mesh*> &meshes);
	void updateLightBuffer(const Light *pLights, U32 numLights, const BufferManager &bufferManager);
	void updateBuffers(const MeshInstance *pInstances, U32 instanceCount, const vx::sorted_vector<const Material*, U32> &materialIndices, const vx::sorted_vector<vx::StringID64, MeshEntry> &meshEntries);

public:
	SceneRenderer();
	~SceneRenderer();

	void initialize(U32 maxLightCount, BufferManager* pBufferManager, vx::StackAllocator *pAllocator);
	void bindBuffers();

	void loadScene(const Scene &scene, const BufferManager &bufferManager, NavMeshRenderer* pNavMeshRenderer);

	U16 getActorGpuIndex();

	const vx::gl::Buffer& getCmdBuffer() const { return m_commandBlock; }
	U32 getMeshInstanceCount() const { return m_meshInstancesCountTotal; }
	const vx::gl::VertexArray& getMeshVao() const { return m_meshVao; }
};