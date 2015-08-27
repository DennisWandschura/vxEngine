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
#include "Vertex.h"
#include "UploadManager.h"
#include "ResourceManager.h"
#include "ResourceView.h"

//#include "ConverterMesh.h"

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
	m_instanceCapacity(0),
	m_geometryHeap()
{
}

MeshManager::~MeshManager()
{
}

bool MeshManager::createHeap(u32 vertexCount, u32 indexCount, u32 instanceCount, ID3D12Device* device)
{
	auto vertexBufferSize = d3d::getAlignedSize(sizeof(Vertex) * vertexCount, 64llu KBYTE);
	auto indexBufferSize = d3d::getAlignedSize(sizeof(u32) * vertexCount, 64llu KBYTE);
	auto instanceBufferSize = d3d::getAlignedSize(sizeof(u32) * vertexCount, 64llu KBYTE);

	return m_geometryHeap.createBufferHeap(vertexBufferSize + indexBufferSize + instanceBufferSize, D3D12_HEAP_TYPE_DEFAULT, device);
}

bool MeshManager::createBuffers(u32 vertexCount, u32 indexCount, u32 instanceCount, ID3D12Device* device)
{
	auto vertexBufferSize = d3d::getAlignedSize(sizeof(Vertex) * vertexCount, 64llu KBYTE);
	auto indexBufferSize = d3d::getAlignedSize(sizeof(u32) * vertexCount, 64llu KBYTE);
	auto instanceBufferSize = d3d::getAlignedSize(sizeof(u32) * vertexCount, 64llu KBYTE);

	d3d::HeapCreateBufferResourceDesc desc[3]=
	{
		{ vertexBufferSize, m_vertexBuffer.getAddressOf(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER },
		{ indexBufferSize, m_indexBuffer.getAddressOf(),D3D12_RESOURCE_STATE_INDEX_BUFFER },
		{ instanceBufferSize, m_drawIdBuffer.getAddressOf(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER }
	};

	if (!m_geometryHeap.createResourceBuffer(desc, 3))
		return false;

	return true;
}

bool MeshManager::initialize(u32 vertexCount, u32 indexCount, u32 instanceCount, d3d::ResourceManager* resourceManager, ID3D12Device* device, vx::StackAllocator* allocator)
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

	d3d::ResourceView meshVertexBufferView;
	meshVertexBufferView.type = d3d::ResourceView::Type::VertexBufferView;
	meshVertexBufferView.vbv = getVertexBufferView();
	resourceManager->insertResourceView("meshVertexBufferView", meshVertexBufferView);

	d3d::ResourceView meshDrawIdBufferView;
	meshDrawIdBufferView.type = d3d::ResourceView::Type::VertexBufferView;
	meshDrawIdBufferView.vbv = getDrawIdBufferView();
	resourceManager->insertResourceView("meshDrawIdBufferView", meshDrawIdBufferView);

	d3d::ResourceView meshIndexBufferView;
	meshIndexBufferView.type = d3d::ResourceView::Type::IndexBufferView;
	meshIndexBufferView.ibv = getIndexBufferView();
	resourceManager->insertResourceView("meshIndexBufferView", meshIndexBufferView);

	m_instanceCapacity = instanceCount;

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

void MeshManager::uploadVertices(const Vertex* vertices, u32 count, u32 offset, UploadManager* uploadMgr)
{
	auto offsetInBytes = sizeof(Vertex) * offset;
	auto sizeInBytes = sizeof(Vertex) * count;

	uploadMgr->pushUploadBuffer((u8*)vertices, m_vertexBuffer.get(), offsetInBytes, sizeInBytes, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	/*D3D12_RANGE range;
	range.Begin = offsetInBytes;
	range.End = offsetInBytes + sizeInBytes;

	Vertex* vertexPtr = nullptr;
	m_vertexBuffer->Map(0, &range, (void**)&vertexPtr);
	for (u32 i = 0; i < count; ++i)
	{
		vertexPtr[i + offset] = vertices[i];
	}
	//vertexPtr = vertexPtr + offset;
	//memcpy(vertexPtr, vertices, sizeInBytes);
	m_vertexBuffer->Unmap(0, &range);*/
}

void MeshManager::uploadIndices(const u32* indices, u32 count, u32 offset, UploadManager* uploadMgr)
{
	auto offsetInBytes = sizeof(u32) * offset;
	auto sizeInBytes = sizeof(u32) * count;

	uploadMgr->pushUploadBuffer((u8*)indices, m_indexBuffer.get(), offsetInBytes, sizeInBytes, D3D12_RESOURCE_STATE_INDEX_BUFFER);

	/*D3D12_RANGE range;
	range.Begin = offsetInBytes;
	range.End = offsetInBytes + sizeInBytes;

	u32* indexPtr = nullptr;
	m_indexBuffer->Map(0, &range, (void**)&indexPtr);
	indexPtr = indexPtr + offset;
	memcpy(indexPtr, indices, sizeInBytes);
	m_indexBuffer->Unmap(0, &range);*/
}

const MeshManager::MeshEntry* MeshManager::addMeshEntry(const vx::StringID &sid, const vx::Mesh &mesh, UploadManager* uploadMgr)
{
	auto meshVertices = mesh.getVertices();
	auto meshVertexCount = mesh.getVertexCount();
	auto meshIndices = mesh.getIndices();
	auto meshIndexCount = mesh.getIndexCount();

	auto vertexOffset = m_vertexCount;
	auto indexOffset = m_indexCount;

	//MeshQ meshQ;
	//ConverterMesh::convertMesh(mesh, &meshQ);

	auto marker = m_scratchAllocator.getMarker();
	auto vertices = (Vertex*)m_scratchAllocator.allocate(sizeof(Vertex) * meshVertexCount);

	for (u32 j = 0; j < meshVertexCount; ++j)
	{
		const float dotCross = vx::dot3(meshVertices[j].tangent, vx::cross(meshVertices[j].bitangent, meshVertices[j].normal));
		float handedness = dotCross > 0.0f ? 1.0f : -1.0f;

		vertices[j].position = vx::float4(meshVertices[j].position, handedness);
		vertices[j].normal = meshVertices[j].normal;
		vertices[j].tangent = meshVertices[j].tangent;

		//auto bitangent = vx::cross(meshVertices[j].normal, meshVertices[j].tangent) * handedness;

		vertices[j].texCoords = meshVertices[j].texCoords;
	}
	uploadVertices(vertices, meshVertexCount, vertexOffset, uploadMgr);

	auto indices = (u32*)m_scratchAllocator.allocate(sizeof(u32) * meshIndexCount);
	for (u32 j = 0; j < meshIndexCount; ++j)
	{
		indices[j] = meshIndices[j] + vertexOffset;
	}
	uploadIndices(indices, meshIndexCount, indexOffset, uploadMgr);

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

void MeshManager::uploadCmd(const DrawIndexedCommand &cmd, d3d::ResourceManager* resourceManager, UploadManager* uploadMgr)
{
	auto drawCmdBuffer = resourceManager->getBuffer(L"drawCmdBuffer");
	D3D12_DRAW_INDEXED_ARGUMENTS cmdArgs;
	cmdArgs.BaseVertexLocation = cmd.baseVertex;
	cmdArgs.IndexCountPerInstance = cmd.indexCount;
	cmdArgs.InstanceCount = cmd.instanceCount;
	cmdArgs.StartIndexLocation = cmd.firstIndex;
	cmdArgs.StartInstanceLocation = cmd.baseInstance;
	uploadMgr->pushUploadBuffer((u8*)&cmdArgs, drawCmdBuffer, cmd.baseInstance, sizeof(D3D12_DRAW_INDEXED_ARGUMENTS), D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
}

void MeshManager::addMeshInstanceImpl(const vx::StringID &instanceSid, const MeshEntry &meshEntry, u16 materialIndex, UploadManager* uploadMgr, DrawIndexedCommand* outCmd)
{
	auto baseInstance = m_instanceCount++;

	DrawIndexedCommand cmd;
	cmd.baseInstance = baseInstance;
	cmd.baseVertex = 0;
	cmd.firstIndex = meshEntry.indexStart;
	cmd.indexCount = meshEntry.indexCount;
	cmd.instanceCount = 1;

	VX_ASSERT((u16)baseInstance == baseInstance);
	u32 drawId = baseInstance | (materialIndex << 16);

	/*u32* drawIdPtr = nullptr;
	m_drawIdBuffer->Map(0, nullptr, (void**)&drawIdPtr);
	drawIdPtr[baseInstance] = drawId;
	m_drawIdBuffer->Unmap(0, nullptr);*/

	auto offset = sizeof(u32)*baseInstance;
	uploadMgr->pushUploadBuffer((u8*)&drawId, m_drawIdBuffer.get(), offset, sizeof(u32), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	m_sortedDrawCommands.insert(instanceSid, cmd);

	*outCmd = cmd;
}

const MeshManager::MeshEntry* MeshManager::tryAddMesh(const vx::StringID &meshSid, const ResourceAspectInterface* resourceAspect, UploadManager* uploadMgr)
{
	auto meshEntry = getMeshEntry(meshSid);
	if (meshEntry == nullptr)
	{
		auto meshFile = resourceAspect->getMesh(meshSid);
		meshEntry = addMeshEntry(meshSid, meshFile->getMesh(), uploadMgr);
	}

	return meshEntry;
}

bool MeshManager::addMeshInstance(const MeshInstance &meshInstance, u16 materialIndex, const ResourceAspectInterface* resourceAspect, d3d::ResourceManager* resourceManager, UploadManager* uploadMgr, DrawIndexedCommand* outCmd)
{
	if (m_instanceCount >= m_instanceCapacity)
		return false;

	auto instanceSid = meshInstance.getNameSid();
	auto it = m_sortedDrawCommands.find(instanceSid);

	if (it == m_sortedDrawCommands.end())
	{
		auto meshSid = meshInstance.getMeshSid();
		auto meshEntry = tryAddMesh(meshSid, resourceAspect, uploadMgr);
		if (!meshEntry)
		{
			return false;
		}

		addMeshInstanceImpl(instanceSid, *meshEntry, materialIndex, uploadMgr, outCmd);
		uploadCmd(*outCmd, resourceManager, uploadMgr);
	}
	else
	{
		*outCmd = *it;
	}

	return true;
}

bool MeshManager::addMeshInstance(const vx::StringID &instanceSid, const vx::StringID &meshSid, u16 materialIndex, ResourceAspectInterface* resourceAspect,
	d3d::ResourceManager* resourceManager, UploadManager* uploadMgr, DrawIndexedCommand* outCmd)
{
	if (m_instanceCount >= m_instanceCapacity)
		return false;

	auto it = m_sortedDrawCommands.find(instanceSid);
	if (it == m_sortedDrawCommands.end())
	{
		auto meshEntry = tryAddMesh(meshSid, resourceAspect, uploadMgr);
		if (!meshEntry)
		{
			return false;
		}

		addMeshInstanceImpl(instanceSid, *meshEntry, materialIndex, uploadMgr, outCmd);
		uploadCmd(*outCmd, resourceManager, uploadMgr);
	}
	else
	{
		*outCmd = *it;
	}

	return true;
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

void MeshManager::updateResourceViews(d3d::ResourceManager* resourceManager)
{
	auto meshVertexBufferView = resourceManager->getResourceView("meshVertexBufferView");
	meshVertexBufferView->vbv = getVertexBufferView();

	auto meshDrawIdBufferView = resourceManager->getResourceView("meshDrawIdBufferView");
	meshDrawIdBufferView->vbv = getDrawIdBufferView();

	auto meshIndexBufferView = resourceManager->getResourceView("meshIndexBufferView");
	meshIndexBufferView->ibv = getIndexBufferView();
}