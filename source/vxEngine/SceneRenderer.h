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

class Scene;
class Material;
class MeshInstance;
struct VertexPNTUV;
class GpuProfiler;
class Font;

namespace gl
{
	class ObjectManager;
}

namespace vx
{
	namespace gl
	{
		class ShaderManager;
	}

	struct Transform;
	struct TransformGpu;
	class Mesh;
}

#include <vxLib/gl/Buffer.h>
#include <vxLib/gl/VertexArray.h>
#include "TextureManager.h"
#include "LightBufferManager.h"
#include <vxLib/Allocator/StackAllocator.h>
#include <vxLib/StringID.h>

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
		vx::sorted_vector<vx::StringID, MeshEntry> m_meshEntries;
		vx::sorted_vector<const MeshInstance*, vx::gl::DrawElementsIndirectCommand> m_instanceCmds;
		vx::sorted_vector<U64, U32> m_texturesGPU;
	};

	vx::gl::Buffer m_commandBlock;
	vx::gl::VertexArray m_meshVao;
	U32 m_meshInstancesCountTotal{ 0 };
	std::unique_ptr<ColdData> m_coldData;

	void createTextures(gl::ObjectManager* objectManager);
	void createMeshDrawIdVbo();
	void createMeshIbo();
	void bindMeshDrawIdVboToVao(vx::gl::VertexArray* vao);
	void createMeshBuffers();

	void createMeshCmdBuffer();
	void createMeshTransformBuffer();
	void createMeshMaterialBuffer();

	void createMaterial(Material* pMaterial);

	void writeMaterialToBuffer(const Material *pMaterial, U32 offset);
	void writeMeshToBuffer(const vx::StringID &meshSid, const vx::Mesh* pMesh, VertexPNTUV* pVertices, U32* pIndices, U32* vertexOffset, U32* indexOffset, U32 *vertexOffsetGpu, U32 *indexOffsetGpu);
	void writeMeshesToVertexBuffer(const vx::StringID* meshSid, const vx::Mesh** pMesh, U32 count, U32 *vertexOffsetGpu, U32 *indexOffsetGpu);
	void writeMeshInstanceIdBuffer(U32 elementId, U32 materialIndex);
	void writeMeshInstanceToCommandBuffer(MeshEntry meshEntry, U32 index, U32 elementId, vx::gl::DrawElementsIndirectCommand* cmd);

	void updateMeshBuffer(const vx::sorted_vector<vx::StringID, const vx::Mesh*> &meshes);
	void updateLightBuffer(const Light *pLights, U32 numLights, const gl::ObjectManager &objectManager);
	void updateBuffers(const MeshInstance *pInstances, U32 instanceCount, const vx::sorted_vector<const Material*, U32> &materialIndices, const vx::sorted_vector<vx::StringID, MeshEntry> &meshEntries);

public:
	SceneRenderer();
	~SceneRenderer();

	void initialize(U32 maxLightCount, gl::ObjectManager* objectManager, vx::StackAllocator *pAllocator);
	bool initializeProfiler(const Font &font, U64 fontTextureHandle,const vx::uint2 &resolution, const vx::gl::ShaderManager &shaderManager, GpuProfiler* gpuProfiler, vx::StackAllocator *pAllocator);

	void bindTransformBuffer();
	void bindMaterialBuffer();
	void bindBuffers();

	void loadScene(const Scene &scene, const gl::ObjectManager &objectManager);
	TextureRef loadTexture(const char* file);

	U16 getActorGpuIndex();

	const vx::gl::Buffer& getCmdBuffer() const { return m_commandBlock; }
	U32 getMeshInstanceCount() const { return m_meshInstancesCountTotal; }
	const vx::gl::VertexArray& getMeshVao() const { return m_meshVao; }
	vx::gl::DrawElementsIndirectCommand getDrawCommand(const MeshInstance* p) const;

	U16 addActorToBuffer(const vx::Transform &transform, const vx::StringID &mesh, const vx::StringID &material, const Scene* pScene);
	void updateTransform(const vx::Transform &t, U32 elementId);
	void updateTransform(const vx::TransformGpu &t, U32 elementId);
	void updateLights(const Light* lights, U32 count);

	U32 getLightCount() const;
};