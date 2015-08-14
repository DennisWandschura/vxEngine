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

#include "MeshManager.h"
#include <d3d12.h>
#include "Vertex.h"
#include <vxEngineLib/MeshFile.h>
#include <vxLib/Graphics/Mesh.h>
#include <vxEngineLib/MeshInstance.h>
#include <vxEngineLib/ResourceAspectInterface.h>
#include "Transform.h"

struct MeshManager::MeshEntry
{
	u32 indexStart;
	u32 indexCount;
};

MeshManager::MeshManager()
	:m_vertexBuffer(),
	m_indexBuffer(),
	m_drawIdBuffer(),
	m_meshEntries(),
	m_vertexCount(0),
	m_indexCount(0),
	m_instanceCount(0),
	m_geometryHeap()
{
}

MeshManager::~MeshManager()
{
}

bool MeshManager::createHeap(u32 vertexCount, u32 indexCount, u32 instanceCount, d3d::Device* device)
{
	auto vertexBufferSize = d3d::getAlignedSize(sizeof(Vertex) * vertexCount, 64 KBYTE);
	auto indexBufferSize = d3d::getAlignedSize(sizeof(u32) * vertexCount, 64 KBYTE);
	auto instanceBufferSize = d3d::getAlignedSize(sizeof(u32) * vertexCount, 64 KBYTE);

	return m_geometryHeap.createBufferHeap(vertexBufferSize+ indexBufferSize+ instanceBufferSize, D3D12_HEAP_TYPE_UPLOAD, device);
}

bool MeshManager::createBuffers(u32 vertexCount, u32 indexCount, u32 instanceCount, d3d::Device* device)
{
	auto vertexBufferSize = d3d::getAlignedSize(sizeof(Vertex) * vertexCount, 64 KBYTE);
	auto indexBufferSize = d3d::getAlignedSize(sizeof(u32) * vertexCount, 64 KBYTE);
	auto instanceBufferSize = d3d::getAlignedSize(sizeof(u32) * vertexCount, 64 KBYTE);

	if (!m_geometryHeap.createResourceBuffer(vertexBufferSize, 0, D3D12_RESOURCE_STATE_GENERIC_READ, m_vertexBuffer.getAddressOf(), device))
		return false;

	if (!m_geometryHeap.createResourceBuffer(indexBufferSize, vertexBufferSize, D3D12_RESOURCE_STATE_GENERIC_READ, m_indexBuffer.getAddressOf(), device))
		return false;

	if (!m_geometryHeap.createResourceBuffer(instanceBufferSize, vertexBufferSize + indexBufferSize, D3D12_RESOURCE_STATE_GENERIC_READ, m_drawIdBuffer.getAddressOf(), device))
		return false;

	return true;
}

bool MeshManager::initialize(u32 vertexCount, u32 indexCount, u32 instanceCount, d3d::Device* device, vx::StackAllocator* allocator)
{
	auto scratchAllocSize = 64 KBYTE;
	auto scratchPtr = allocator->allocate(scratchAllocSize);
	if (scratchPtr == nullptr)
		return false;

	if (!createHeap(vertexCount, indexCount, instanceCount, device))
		return false;

	if (!createBuffers(vertexCount, indexCount, instanceCount, device))
		return false;

	m_scratchAllocator = vx::StackAllocator(scratchPtr, scratchAllocSize);

	return true;
}

void MeshManager::shutdown()
{
	m_scratchAllocator.release();

	m_drawIdBuffer.destroy();
	m_indexBuffer.destroy();
	m_vertexBuffer.destroy();
	m_geometryHeap.destroy();

	m_meshEntries.clear();
	m_vertexCount = 0;
	m_indexCount = 0;
	m_instanceCount = 0;
}

void MeshManager::uploadVertices(const Vertex* vertices, u32 count, u32 offset)
{
	auto offsetInBytes = sizeof(Vertex) * offset;
	auto sizeInBytes = sizeof(Vertex) * count;

	D3D12_RANGE range;
	range.Begin = offsetInBytes;
	range.End = offsetInBytes + sizeInBytes;

	Vertex* vertexPtr = nullptr;
	m_vertexBuffer->Map(0, &range, (void**)&vertexPtr);
	vertexPtr = vertexPtr + offset;
	memcpy(vertexPtr, vertices, sizeInBytes);
	m_vertexBuffer->Unmap(0, &range);
}

void MeshManager::uploadIndices(const u32* indices, u32 count, u32 offset)
{
	auto offsetInBytes = sizeof(u32) * offset;
	auto sizeInBytes = sizeof(u32) * count;

	D3D12_RANGE range;
	range.Begin = offsetInBytes;
	range.End = offsetInBytes + sizeInBytes;

	u32* indexPtr = nullptr;
	m_indexBuffer->Map(0, &range, (void**)&indexPtr);
	indexPtr = indexPtr + offset;
	memcpy(indexPtr, indices, sizeInBytes);
	m_indexBuffer->Unmap(0, &range);
}

const MeshManager::MeshEntry* MeshManager::addMeshEntry(const vx::StringID &sid, const vx::Mesh &mesh)
{
	auto meshVertices = mesh.getVertices();
	auto meshVertexCount = mesh.getVertexCount();
	auto meshIndices = mesh.getIndices();
	auto meshIndexCount = mesh.getIndexCount();

	auto vertexOffset = m_vertexCount;
	auto indexOffset = m_indexCount;

	auto marker = m_scratchAllocator.getMarker();
	auto vertices = (Vertex*)m_scratchAllocator.allocate(sizeof(Vertex) * meshVertexCount);

	for (u32 j = 0; j < meshVertexCount; ++j)
	{
		vertices[j].position = meshVertices[j].position;
		vertices[j].texCoords = meshVertices[j].texCoords;
	}
	uploadVertices(vertices, meshVertexCount, vertexOffset);

	auto indices = (u32*)m_scratchAllocator.allocate(sizeof(u32) * meshIndexCount);
	for (u32 j = 0; j < meshIndexCount; ++j)
	{
		indices[j] = meshIndices[j] + vertexOffset;
	}
	uploadIndices(indices, meshIndexCount, indexOffset);

	m_scratchAllocator.clear(marker);

	MeshEntry entry;
	entry.indexStart = indexOffset;
	entry.indexCount = meshIndexCount;

	auto it = m_meshEntries.insert(sid, entry);
	const MeshEntry* result = &(*it);

	m_vertexCount += meshVertexCount;
	m_indexCount += meshIndexCount;

	return result;
}

const MeshManager::MeshEntry* MeshManager::getMeshEntry(const vx::StringID &sid) const
{
	auto it = m_meshEntries.find(sid);

	const MeshEntry* result = (it != m_meshEntries.end()) ? &(*it) : nullptr;

	return result;
}

void MeshManager::addMeshInstance(const MeshInstance &meshInstance, const ResourceAspectInterface* resourceAspect, DrawIndexedCommand* outCmd)
{
	auto meshSid = meshInstance.getMeshSid();
	auto meshEntry = getMeshEntry(meshSid);
	if (meshEntry == nullptr)
	{
		auto meshFile = resourceAspect->getMesh(meshSid);
		meshEntry = addMeshEntry(meshSid, meshFile->getMesh());
	}

	auto baseInstance = m_instanceCount++;

	DrawIndexedCommand cmd;
	cmd.baseInstance = baseInstance;
	cmd.baseVertex = 0;
	cmd.firstIndex = meshEntry->indexStart;
	cmd.indexCount = meshEntry->indexCount;
	cmd.instanceCount = 1;

	u32* indexPtr = nullptr;
	m_drawIdBuffer->Map(0, nullptr, (void**)&indexPtr);
	indexPtr[baseInstance] = baseInstance;
	m_drawIdBuffer->Unmap(0, nullptr);

	*outCmd = cmd;

	//auto meshTransform = meshInstance.getTransform();

	//Transform transform;
	//transform.translation = vx::float4(meshTransform.m_translation, 1);

	//auto transformOffsetInBytes = sizeof(Transform) * baseInstance;
}

D3D12_VERTEX_BUFFER_VIEW MeshManager::getVertexBufferView()
{
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(Vertex) * m_vertexCount;
	vertexBufferView.StrideInBytes = sizeof(Vertex);

	return vertexBufferView;
}

D3D12_VERTEX_BUFFER_VIEW MeshManager::getDrawIdBufferView()
{
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewMeshIndices;
	vertexBufferViewMeshIndices.BufferLocation = m_drawIdBuffer->GetGPUVirtualAddress();
	vertexBufferViewMeshIndices.SizeInBytes = sizeof(u32) * m_instanceCount;
	vertexBufferViewMeshIndices.StrideInBytes = sizeof(u32);

	return vertexBufferViewMeshIndices;
}

D3D12_INDEX_BUFFER_VIEW MeshManager::getIndexBufferView()
{
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
	indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	indexBufferView.SizeInBytes = sizeof(u32) * m_indexCount;

	return indexBufferView;
}