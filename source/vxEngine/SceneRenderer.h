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
	class MeshFile;
}

#include <vxLib/gl/Buffer.h>
#include <vxLib/gl/VertexArray.h>
#include "TextureManager.h"
#include "LightBufferManager.h"
#include <vxLib/Allocator/StackAllocator.h>
#include <vxLib/StringID.h>

struct MeshEntry
{
	u32 firstIndex;
	u32 indexCount;
};

class SceneRenderer
{
	static const auto s_maxVerticesStatic = 500000u;
	static const auto s_maxVerticesDynamic = 200000u;

	static const auto s_maxIndicesStatic = 40000u;
	static const auto s_maxIndicesDynamic = 20000u;

	static const auto s_maxMaterials = 100u;
	static const auto s_meshMaxInstancesStatic = 100u;
	static const auto s_meshMaxInstancesDynamic = 50u;

	struct ColdData;
	struct WriteMeshesToGpuBufferDesc;
	struct WriteMeshToBufferDesc;

	gl::ObjectManager* m_pObjectManager;
	u32 m_staticMeshInstanceCount{ 0 };
	u32 m_dynamicMeshInstanceCount{ 0 };
	u32 m_meshInstanceCount{ 0 };
	std::unique_ptr<ColdData> m_coldData;

	void createTextures();
	void createMeshDrawIdVbo();
	void createMeshIbo();
	void createMeshBuffers();

	void createMeshCmdBuffer();
	void createMeshTransformBuffer();
	void createMeshMaterialBuffer();

	void createMaterial(Material* pMaterial);

	void setMeshParamBufferValue(u32 count);

	void addMeshInstanceToBuffers(const MeshInstance &instance, const vx::gl::Buffer* cmdBuffer, const MeshEntry &meshEntry, u16 elementId, u32 materialIndex);
	void writeMaterialToBuffer(const Material *pMaterial, u32 offset);
	void writeMeshToBuffer(const WriteMeshToBufferDesc &desc);
	void writeMeshesToGpuBuffer(const WriteMeshesToGpuBufferDesc &desc);
	void writeMeshInstanceIdBuffer(u32 elementId, u32 materialIndex) const;
	void writeMeshInstanceToCommandBuffer(MeshEntry meshEntry, u32 index, u32 elementId, vx::gl::DrawElementsIndirectCommand* cmd, const vx::gl::Buffer* cmdBuffer);

	void updateStaticMeshBuffer(const vx::sorted_vector<vx::StringID, const vx::MeshFile*> &meshes);
	void updateLightBuffer(const Light *pLights, u32 numLights, const gl::ObjectManager &objectManager);
	void updateBuffersWithMeshInstance(const MeshInstance &instance, u16 elementId, const vx::gl::Buffer* cmdBuffer);
	void updateBuffers(const void *pInstances, u32 instanceCount);

public:
	SceneRenderer();
	~SceneRenderer();

	void initialize(u32 maxLightCount, gl::ObjectManager* objectManager, vx::StackAllocator *pAllocator);
	bool initializeProfiler(const Font &font, u64 fontTextureHandle,const vx::uint2 &resolution, const vx::gl::ShaderManager &shaderManager, GpuProfiler* gpuProfiler, vx::StackAllocator *pAllocator);

	void loadScene(const void* scene, const gl::ObjectManager &objectManager);
	TextureRef loadTexture(const char* file);

	u32 getMaterialIndex(const Material* material) const;
	bool setMeshInstanceMaterial(const vx::StringID &sid, const Material* material) const;

	u16 getActorGpuIndex();

	u32 getMeshInstanceCount() const { return m_staticMeshInstanceCount + m_dynamicMeshInstanceCount; }
	vx::gl::DrawElementsIndirectCommand getDrawCommand(const vx::StringID &sid) const;

	u16 addActorToBuffer(const vx::Transform &transform, const vx::StringID &mesh, const vx::StringID &material, const Scene* pScene, vx::gl::DrawElementsIndirectCommand* drawCmd, u32* cmdIndex);
	void updateTransform(const vx::Transform &t, u32 elementId);
	void updateTransform(const vx::TransformGpu &t, u32 elementId);
	void updateLights(const Light* lights, u32 count);

	u32 getLightCount() const;

	const MeshEntry* getMeshEntries() const;
	u32 getMeshEntryCount() const;

	void editorAddMeshInstance(const MeshInstance &instance);
	bool editorRemoveStaticMeshInstance(const vx::StringID &sid);
};