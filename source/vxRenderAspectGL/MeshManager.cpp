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
#include "gl/ObjectManager.h"
#include <vxGL/VertexArray.h>
#include "Vertex.h"
#include <vxEngineLib/MeshInstance.h>
#include <vxEngineLib/ResourceAspectInterface.h>
#include <vxEngineLib/MeshFile.h>
#include <vxEngineLib/GpuFunctions.h>

MeshManager::MeshManager()
	:m_vertexBuffer(),
	m_indexBuffer(),
	m_drawIdBuffer(),
	m_sizeVertices(0),
	m_sizeIndices(0),
	m_sizeInstances(0),
	m_capacityVertices(0),
	m_capacityIndices(0),
	m_capacityInstances(0),
	m_objectManager(nullptr),
	m_meshEntries(),
	m_sortedDrawCommands()
{

}

MeshManager::~MeshManager()
{

}

void MeshManager::initialize(u32 maxInstances, u32 maxVertices, u32 maxIndices, gl::ObjectManager* objectManager)
{
	m_objectManager = objectManager;

	m_capacityInstances = maxInstances;
	m_capacityVertices = maxVertices;
	m_capacityIndices = maxIndices;

	createBuffers();
}

void MeshManager::shutdown()
{
	m_vertexBuffer.destroy();
	m_indexBuffer.destroy();
	m_drawIdBuffer.destroy();
}

void MeshManager::createVertexBuffer()
{
	vx::gl::BufferDescription desc;
	desc.bufferType = vx::gl::BufferType::Array_Buffer;
	desc.flags = vx::gl::BufferStorageFlags::Write;
	desc.immutable = 1;
	desc.size = sizeof(VertexPNTUV) * m_capacityVertices;

	m_vertexBuffer.create(desc);
}

void MeshManager::createIndexBuffer()
{
	vx::gl::BufferDescription desc;
	desc.bufferType = vx::gl::BufferType::Element_Array_Buffer;
	desc.flags = vx::gl::BufferStorageFlags::Write;
	desc.immutable = 1;
	desc.size = sizeof(u32) * m_capacityIndices;

	m_indexBuffer.create(desc);
}

void MeshManager::createDrawIdBuffer()
{
	vx::gl::BufferDescription desc;
	desc.bufferType = vx::gl::BufferType::Array_Buffer;
	desc.flags = vx::gl::BufferStorageFlags::Write;
	desc.immutable = 1;
	desc.size = sizeof(u32) * m_capacityInstances;

	m_drawIdBuffer.create(desc);
}

void MeshManager::createCmdBuffer()
{
	vx::gl::BufferDescription desc;
	desc.bufferType = vx::gl::BufferType::Draw_Indirect_Buffer;
	desc.flags = vx::gl::BufferStorageFlags::Write;
	desc.immutable = 1;
	desc.size = sizeof(vx::gl::DrawElementsIndirectCommand) * m_capacityInstances;
	desc.pData = nullptr;

	m_objectManager->createBuffer("meshCmdBuffer", desc);

	u32 count = 0;
	desc.bufferType = vx::gl::BufferType::Parameter_Buffer;
	desc.flags = vx::gl::BufferStorageFlags::Write | vx::gl::BufferStorageFlags::Dynamic_Storage;
	desc.size = sizeof(u32);
	desc.pData = &count;
	m_objectManager->createBuffer("meshParamBuffer", desc);
}

void MeshManager::createTransformBuffer()
{
	vx::gl::BufferDescription desc;
	desc.bufferType = vx::gl::BufferType::Shader_Storage_Buffer;
	desc.flags = vx::gl::BufferStorageFlags::Write;
	desc.immutable = 1;
	desc.size = sizeof(vx::TransformGpu) * m_capacityInstances;

	m_objectManager->createBuffer("transformBuffer", desc);
}

void MeshManager::createBuffers()
{
	createVertexBuffer();
	createIndexBuffer();
	createDrawIdBuffer();
	createCmdBuffer();
	createTransformBuffer();

	auto vaoSid = m_objectManager->createVertexArray("meshVao");
	auto vao = m_objectManager->getVertexArray(vaoSid);

	/*
	vx::float4 position;
	vx::float3 normal;
	vx::float3 tangent;
	vx::float2 uv;
	u32 drawid
	*/

	vao->enableArrayAttrib(0);
	vao->enableArrayAttrib(1);
	vao->enableArrayAttrib(2);
	vao->enableArrayAttrib(3);
	vao->enableArrayAttrib(4);

	vao->arrayAttribBinding(0, 0);
	vao->arrayAttribBinding(1, 0);
	vao->arrayAttribBinding(2, 0);
	vao->arrayAttribBinding(3, 0);
	vao->arrayAttribBinding(4, 1);

	vao->arrayAttribFormatF(0, 4, 0, offsetof(VertexPNTUV, position));
	vao->arrayAttribFormatF(1, 3, 0, offsetof(VertexPNTUV, normal));
	vao->arrayAttribFormatF(2, 3, 0, offsetof(VertexPNTUV, tangent));
	vao->arrayAttribFormatF(3, 2, 0, offsetof(VertexPNTUV, uv));

	vao->arrayAttribFormatI(4, 1, vx::gl::DataType::Unsigned_Int, 0);

	vao->arrayBindingDivisor(1, 1);

	vao->bindIndexBuffer(m_indexBuffer);
	vao->bindVertexBuffer(m_vertexBuffer, 0, 0, sizeof(VertexPNTUV));
	vao->bindVertexBuffer(m_drawIdBuffer, 1, 0, sizeof(u32));
}

void MeshManager::addMeshInstances(const MeshManagerMeshInstanceDesc &desc)
{
	auto count = desc.instanceCount;
	auto instances = desc.instances;
	auto outDrawIds = desc.outDrawIds;
	auto resourceAspect = desc.resourceAspect;
	auto materialIndices = desc.materialIndices;

	for (u32 i = 0;i < count; ++i)
	{
		outDrawIds[i] = addMeshInstance(instances[i], materialIndices[i], resourceAspect);
	}
}

u32 MeshManager::addMeshInstance(const MeshInstance &instance, u16 materialIndex, ResourceAspectInterface* resourceAspect)
{
	auto meshSid = instance.getMeshSid();
	auto transform = instance.getTransform();

	return addMeshInstance(instance.getNameSid(), transform, meshSid, materialIndex, resourceAspect);
}

void MeshManager::uploadCmd(const vx::gl::DrawElementsIndirectCommand &drawCmd)
{
	auto drawId = drawCmd.baseInstance;

	auto cmdBuffer = m_objectManager->getBuffer("meshCmdBuffer");
	auto mappedCmdBuffer = cmdBuffer->mapRange<vx::gl::DrawElementsIndirectCommand>(sizeof(vx::gl::DrawElementsIndirectCommand) * drawId, sizeof(vx::gl::DrawElementsIndirectCommand), vx::gl::MapRange::Write);
	*mappedCmdBuffer = drawCmd;
	mappedCmdBuffer.unmap();
}

u32 MeshManager::addMeshInstance(const vx::StringID &instanceSid, const vx::Transform &transform, const vx::StringID &meshSid, u16 materialIndex, ResourceAspectInterface* resourceAspect)
{
	auto it = m_meshEntries.find(meshSid);
	if (it == m_meshEntries.end())
	{
		auto meshFile = resourceAspect->getMesh(meshSid);
		auto &mesh = meshFile->getMesh();
		auto meshEntry = addMeshToGpu(mesh);

		it = m_meshEntries.insert(meshSid, meshEntry);
	}

	auto drawId = m_sizeInstances;

	vx::gl::DrawElementsIndirectCommand drawCmd{};
	drawCmd.firstIndex = it->first;
	drawCmd.count = it->count;
	drawCmd.instanceCount = 1;
	drawCmd.baseInstance = drawId;

	m_sortedDrawCommands.insert(instanceSid, drawCmd);

	uploadCmd(drawCmd);

	auto packedData = drawId | (materialIndex << 16);
	auto mappedDrawIdBuffer = m_drawIdBuffer.mapRange<u32>(sizeof(u32) * drawId, sizeof(u32), vx::gl::MapRange::Write);
	*mappedDrawIdBuffer = packedData;
	mappedDrawIdBuffer.unmap();

	updateTransform(transform, drawId);

	++m_sizeInstances;

	auto buffer = m_objectManager->getBuffer("meshParamBuffer");
	buffer->subData(0, sizeof(u32), &m_sizeInstances);

	return drawId;
}

MeshManager::MeshEntry MeshManager::addMeshToGpu(const vx::Mesh &mesh)
{
	auto vertexCount = mesh.getVertexCount();
	auto vertices = mesh.getVertices();
	auto vertexOffset = m_sizeVertices;

	auto mappedVertexBuffer = m_vertexBuffer.mapRange<VertexPNTUV>(sizeof(VertexPNTUV) * vertexOffset, sizeof(VertexPNTUV) * vertexCount, vx::gl::MapRange::Write);
	for (u32 i = 0; i < vertexCount; ++i)
	{
		auto vertex = vertices[i];

		mappedVertexBuffer[i].position = vx::float4(vertex.position, 1);
		mappedVertexBuffer[i].normal = vertex.normal;
		mappedVertexBuffer[i].tangent = vertex.tangent;
		mappedVertexBuffer[i].uv = vertex.texCoords;
	}
	mappedVertexBuffer.unmap();

	auto indexCount = mesh.getIndexCount();
	auto indices = mesh.getIndices();
	auto indexOffset = m_sizeIndices;

	auto mappedIndexBuffer = m_indexBuffer.mapRange<u32>(sizeof(u32) * indexOffset, sizeof(u32) * indexCount, vx::gl::MapRange::Write);
	for (u32 i = 0; i < indexCount; ++i)
	{
		mappedIndexBuffer[i] = indices[i] + vertexOffset;
	}
	mappedIndexBuffer.unmap();

	MeshEntry meshEntry;
	meshEntry.first = indexOffset;
	meshEntry.count = indexCount;

	m_sizeVertices += vertexCount;
	m_sizeIndices += indexCount;

	return meshEntry;
}

void MeshManager::addMesh(const vx::Mesh &mesh)
{
	addMeshToGpu(mesh);
}

void MeshManager::addMeshes(const vx::Mesh* meshes, u32 count)
{
	for (u32 i = 0;i < count; ++i)
	{
		addMeshToGpu(meshes[i]);
	}
}

void MeshManager::updateTransform(const vx::Transform &transform, u32 index)
{
	vx::TransformGpu transformGpu;
	transformGpu.translation = transform.m_translation;
	transformGpu.scaling = transform.m_scaling;
	transformGpu.packedQRotation = GpuFunctions::packQRotation(vx::loadFloat4(transform.m_qRotation));

	updateTransform(transformGpu, index);
}

void MeshManager::updateTransform(const vx::TransformGpu &transform, u32 index)
{
	auto transformBuffer = m_objectManager->getBuffer("transformBuffer");
	auto mappedBuffer = transformBuffer->mapRange<vx::TransformGpu>(sizeof(vx::TransformGpu) * index, sizeof(vx::TransformGpu), vx::gl::MapRange::Write);
	*mappedBuffer = transform;
	mappedBuffer.unmap();
}

bool MeshManager::getDrawCommand(const vx::StringID &instanceSid, vx::gl::DrawElementsIndirectCommand* cmd)
{
	bool result = false;

	auto it = m_sortedDrawCommands.find(instanceSid);
	if (it != m_sortedDrawCommands.end())
	{
		*cmd = *it;
		result = true;
	}

	return result;
}

void MeshManager::setMaterial(const vx::StringID &instanceSid, u32 materialIndex)
{
	vx::gl::DrawElementsIndirectCommand cmd;
	auto found = getDrawCommand(instanceSid, &cmd);
	if (found)
	{
		auto drawId = cmd.baseInstance;

		auto packedData = drawId | (materialIndex << 16);

		auto mappedDrawIdBuffer = m_drawIdBuffer.mapRange<u32>(sizeof(u32) * drawId, sizeof(u32), vx::gl::MapRange::Write);
		*mappedDrawIdBuffer = packedData;
		mappedDrawIdBuffer.unmap();
	}
}

void MeshManager::setMesh(const vx::StringID &instanceSid, const vx::StringID &meshSid, ResourceAspectInterface* resourceAspect)
{
	auto itMesh = m_meshEntries.find(meshSid);
	auto itCmd = m_sortedDrawCommands.find(instanceSid);

	if (itMesh == m_meshEntries.end())
	{
		auto meshFile = resourceAspect->getMesh(meshSid);
		auto &mesh = meshFile->getMesh();
		auto meshEntry = addMeshToGpu(mesh);

		itMesh = m_meshEntries.insert(meshSid, meshEntry);
	}

	if (itCmd != m_sortedDrawCommands.end() && itMesh != m_meshEntries.end())
	{
		itCmd->firstIndex = itMesh->first;
		itCmd->count = itMesh->count;

		auto drawCmd = *itCmd;

		uploadCmd(drawCmd);
	}
}