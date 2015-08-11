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

class MeshInstance;
class ResourceAspectInterface;

namespace vx
{
	struct StringID;
	class Mesh;
	struct Transform;
	struct TransformGpu;
}

namespace gl
{
	class ObjectManager;
}

#include <vxLib/types.h>
#include <vxGL/Buffer.h>
#include <vxLib/Container/sorted_vector.h>

struct MeshManagerMeshInstanceDesc
{
	const MeshInstance* instances;
	ResourceAspectInterface* resourceAspect;
	u16* materialIndices;
	u32* outDrawIds;
	u32 instanceCount;
};

class MeshManager
{
	struct MeshEntry
	{
		u32 first;
		u32 count;
	};

	vx::gl::Buffer m_vertexBuffer;
	vx::gl::Buffer m_indexBuffer;
	vx::gl::Buffer m_drawIdBuffer;
	u32 m_sizeVertices;
	u32 m_sizeIndices;
	u32 m_sizeInstances;
	u32 m_capacityVertices;
	u32 m_capacityIndices;
	u32 m_capacityInstances;
	gl::ObjectManager* m_objectManager;
	vx::sorted_vector<vx::StringID, MeshEntry> m_meshEntries;
	vx::sorted_vector<vx::StringID, vx::gl::DrawElementsIndirectCommand> m_sortedDrawCommands;

	void createVertexBuffer();
	void createIndexBuffer();
	void createDrawIdBuffer();
	void createCmdBuffer();
	void createTransformBuffer();
	void createBuffers();

	MeshEntry addMeshToGpu(const vx::Mesh &mesh);
	void uploadCmd(const vx::gl::DrawElementsIndirectCommand &cmd);

public:
	MeshManager();
	~MeshManager();

	void initialize(u32 maxInstances, u32 maxVertices, u32 maxIndices, gl::ObjectManager* objectManager);
	void shutdown();

	u32 addMeshInstance(const MeshInstance &instance, u16 materialIndex, ResourceAspectInterface* resourceAspect);
	void addMeshInstances(const MeshManagerMeshInstanceDesc &desc);
	u32 addMeshInstance(const vx::StringID &instanceSid, const vx::Transform &transform, const vx::StringID &meshSid, u16 materialIndex, ResourceAspectInterface* resourceAspect);

	void addMesh(const vx::Mesh &mesh);
	void addMeshes(const vx::Mesh* meshes, u32 count);

	void updateTransform(const vx::Transform &transform, u32 index);
	void updateTransform(const vx::TransformGpu &transform, u32 index);

	bool getDrawCommand(const vx::StringID &instanceSid, vx::gl::DrawElementsIndirectCommand* cmd);

	void setMaterial(const vx::StringID &instanceSid, u32 materialIndex);
	void setMesh(const vx::StringID &instanceSid, const vx::StringID &meshSid);
};