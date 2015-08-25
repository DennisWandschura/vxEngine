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

struct ID3D12Resource;
struct D3D12_VERTEX_BUFFER_VIEW;
struct D3D12_INDEX_BUFFER_VIEW;

class MeshInstance;
struct Vertex;
class ResourceAspectInterface;
class UploadManager;

namespace vx
{
	class MeshFile;
	class Mesh;
}

#include <vxLib/Container/sorted_vector.h>
#include <vxLib/StringID.h>
#include "d3d.h"
#include "Heap.h"
#include <vxLib/Allocator/StackAllocator.h>

struct DrawIndexedCommand
{
	u32 indexCount;
	u32 instanceCount;
	u32 firstIndex;
	u32 baseVertex;
	u32 baseInstance;
};

class MeshManager
{
	struct MeshEntry;

	d3d::Object<ID3D12Resource> m_vertexBuffer;
	d3d::Object<ID3D12Resource> m_indexBuffer;
	d3d::Object<ID3D12Resource> m_drawIdBuffer;
	vx::sorted_vector<vx::StringID, MeshEntry> m_meshEntries;
	u32 m_vertexCount;
	u32 m_indexCount;
	u32 m_instanceCount;
	vx::StackAllocator m_scratchAllocator;
	d3d::Heap m_geometryHeap;
	vx::sorted_vector<vx::StringID, DrawIndexedCommand> m_sortedDrawCommands;

	bool createHeap(u32 vertexCount, u32 indexCount, u32 instanceCount, ID3D12Device* device);
	bool createBuffers(u32 vertexCount, u32 indexCount, u32 instanceCount, ID3D12Device* device);

	void uploadVertices(const Vertex* vertices, u32 count, u32 offset, UploadManager* uploadMgr);
	void uploadIndices(const u32* indices, u32 count, u32 offset, UploadManager* uploadMgr);

	const MeshEntry* addMeshEntry(const vx::StringID &sid, const vx::Mesh &mesh, UploadManager* uploadMgr);
	const MeshEntry* getMeshEntry(const vx::StringID &sid) const;

	void addMeshInstanceImpl(const vx::StringID &instanceSid, const MeshEntry &meshEntry, u16 materialIndex, UploadManager* uploadMgr, DrawIndexedCommand* outCmd);

public:
	MeshManager();
	~MeshManager();

	bool initialize(u32 vertexCount, u32 indexCount, u32 instanceCount, ID3D12Device* device, vx::StackAllocator* allocator);
	void shutdown();

	void addMeshInstance(const MeshInstance &meshInstance, u16 materialIndex, const ResourceAspectInterface* resourceAspect, UploadManager* uploadMgr, DrawIndexedCommand* cmd);
	void addMeshInstance(const vx::StringID &instanceSid, const vx::StringID &meshSid, u16 materialIndex, ResourceAspectInterface* resourceAspect, UploadManager* uploadMgr, DrawIndexedCommand* outCmd);

	d3d::Object<ID3D12Resource>& getVertexBuffer() { return m_vertexBuffer;}
	d3d::Object<ID3D12Resource>& getIndexBuffer() { return m_indexBuffer; }
	d3d::Object<ID3D12Resource>& getDrawIdBuffer() { return m_drawIdBuffer; }

	u32 getInstanceCount() const { return m_instanceCount; }

	D3D12_VERTEX_BUFFER_VIEW getVertexBufferView();
	D3D12_VERTEX_BUFFER_VIEW getDrawIdBufferView();

	D3D12_INDEX_BUFFER_VIEW getIndexBufferView();
};