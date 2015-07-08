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
class FileAspectInterface;

namespace vx
{
	struct StringID;
	class Mesh;
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
	FileAspectInterface* fileAspect;
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
	vx::gl::Buffer m_transformBuffer;
	u32 m_sizeVertices;
	u32 m_sizeIndices;
	u32 m_sizeInstances;
	u32 m_capacityVertices;
	u32 m_capacityIndices;
	u32 m_capacityInstances;
	gl::ObjectManager* m_objectManager;
	vx::sorted_vector<vx::StringID, MeshEntry> m_meshEntries;

	void createVertexBuffer();
	void createIndexBuffer();
	void createDrawIdBuffer();
	void createCmdBuffer();
	void createTransformBuffer();
	void createBuffers();

	MeshEntry addMeshToGpu(const vx::Mesh &mesh);

public:
	MeshManager();
	~MeshManager();

	void initialize(u32 maxInstances, u32 maxVertices, u32 maxIndices);
	void shutdown();

	u32 addMeshInstance(const MeshInstance &instance, u16 materialIndex, FileAspectInterface* fileAspect);
	void addMeshInstances(const MeshManagerMeshInstanceDesc &desc);

	void addMesh(const vx::Mesh &mesh);
	void addMeshes(const vx::Mesh* meshes, u32 count);
};