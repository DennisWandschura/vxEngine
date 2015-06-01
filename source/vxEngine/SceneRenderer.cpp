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
#include "SceneRenderer.h"
#include "Scene.h"
#include "Vertex.h"
#include "GpuStructs.h"
#include "Transform.h"
#include "GpuFunctions.h"
#include "Locator.h"
#include "FileAspect.h"
#include <vxLib/ScopeGuard.h>
#include "MeshInstance.h"
#include "gl/ObjectManager.h"
#include "gl/BufferBindingManager.h"
#include "Light.h"
#include <vxLib/gl/StateManager.h>
#include <vxLib/gl/gl.h>
#include "TextureFile.h"
#include "GpuProfiler.h"
#include <vxLib/gl/ShaderManager.h>
#include <vxEngineLib/MeshFile.h>
#include <vxLib/algorithm.h>
#include "CpuProfiler.h"

struct SceneRenderer::ColdData
{
	vx::gl::Buffer m_meshVbo;
	vx::gl::Buffer m_meshIdVbo;
	vx::gl::Buffer m_meshIbo;

	u32 m_meshVertexCount{ 0 };
	u32 m_meshIndexCount{ 0 };
	u32 m_materialCount{ 0 };

	LightBufferManager m_lightBufferManager;
	u32 m_dynamicMeshVertexCount{ 0 };
	u32 m_dynamicMeshIndexCount{ 0 };
	vx::StackAllocator m_scratchAllocator;
	TextureManager m_textureManager;
	vx::sorted_vector<const Material*, u32> m_materialIndices;
	vx::sorted_vector<vx::StringID, MeshEntry> m_meshEntries;
	vx::sorted_vector<const MeshInstance*, vx::gl::DrawElementsIndirectCommand> m_instanceCmds;
	vx::sorted_vector<u64, u32> m_texturesGPU;
};

struct SceneRenderer::WriteMeshesToGpuBufferDesc
{
	const vx::StringID* meshSid;
	const vx::MeshFile** pMeshFiles; u32 count;
	u32 *vertexOffsetGpu;
	u32 *indexOffsetGpu;
	vx::gl::Buffer* vbo;
	vx::gl::Buffer* ibo;
	vx::sorted_vector<vx::StringID, MeshEntry>* meshEntries;
};

struct SceneRenderer::WriteMeshToBufferDesc
{
	vx::StringID meshSid;
	const vx::MeshFile* pMeshFile; 
	VertexPNTUV* pVertices;
	u32* pIndices;
	u32* vertexOffset;
	u32* indexOffset;
	u32 *vertexOffsetGpu;
	u32 *indexOffsetGpu;
	vx::sorted_vector<vx::StringID, MeshEntry>* meshEntries;
};

SceneRenderer::SceneRenderer()
	:m_pObjectManager(nullptr)
{

}

SceneRenderer::~SceneRenderer()
{

}

void SceneRenderer::createTextures()
{
	m_coldData->m_textureManager.createBucket(1, vx::ushort3(1024, 1024, 10), 1, vx::gl::TextureType::Texture_2D_Array, vx::gl::TextureFormat::SRGBA8);
	m_coldData->m_textureManager.createBucket(1, vx::ushort3(1024, 1024, 10), 1, vx::gl::TextureType::Texture_2D_Array, vx::gl::TextureFormat::SRGB8);
	m_coldData->m_textureManager.createBucket(1, vx::ushort3(1024, 1024, 10), 1, vx::gl::TextureType::Texture_2D_Array, vx::gl::TextureFormat::RGB8);

	const auto numHandles = 3u;
	u64 handles[numHandles];
	handles[0] = m_coldData->m_textureManager.createTexture(vx::ushort3(1024, 1024, 10), 1, vx::gl::TextureType::Texture_2D_Array, vx::gl::TextureFormat::SRGBA8);
	handles[1] = m_coldData->m_textureManager.createTexture(vx::ushort3(1024, 1024, 10), 1, vx::gl::TextureType::Texture_2D_Array, vx::gl::TextureFormat::RGB8);
	handles[2] = m_coldData->m_textureManager.createTexture(vx::ushort3(1024, 1024, 10), 1, vx::gl::TextureType::Texture_2D_Array, vx::gl::TextureFormat::SRGB8);

	for (u32 i = 0; i < numHandles; ++i)
	{
		m_coldData->m_texturesGPU.insert(handles[i], i);
	}

	vx::gl::BufferDescription desc;
	desc.bufferType = vx::gl::BufferType::Shader_Storage_Buffer;
	desc.size = sizeof(u64) * numHandles;
	desc.immutable = 1;
	desc.pData = handles;

	m_pObjectManager->createBuffer("TextureBuffer", desc);
}

void SceneRenderer::createMeshDrawIdVbo()
{
	vx::gl::BufferDescription idVboDesc;
	idVboDesc.bufferType = vx::gl::BufferType::Array_Buffer;
	idVboDesc.flags = vx::gl::BufferStorageFlags::Write;
	idVboDesc.immutable = 1;

	idVboDesc.size = sizeof(u32) * (s_meshMaxInstancesStatic + s_meshMaxInstancesDynamic);
	m_coldData->m_meshIdVbo.create(idVboDesc);
}

void SceneRenderer::createMeshIbo()
{
	vx::gl::BufferDescription iboDesc;
	iboDesc.bufferType = vx::gl::BufferType::Element_Array_Buffer;
	iboDesc.flags = vx::gl::BufferStorageFlags::Write;
	iboDesc.immutable = 1;

	iboDesc.size = sizeof(u32) * (s_maxIndicesStatic + s_maxIndicesDynamic);
	m_coldData->m_meshIbo.create(iboDesc);
}

void SceneRenderer::createMeshBuffers()
{
	auto meshVaoSid = m_pObjectManager->createVertexArray("meshVao");
	auto meshVao = m_pObjectManager->getVertexArray(meshVaoSid);

	{
		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Array_Buffer;
		desc.flags = vx::gl::BufferStorageFlags::Write;
		desc.immutable = 1;
		desc.pData =nullptr;

		desc.size = sizeof(VertexPNTUV) * (s_maxVerticesStatic + s_maxVerticesDynamic);
		m_coldData->m_meshVbo.create(desc);
	}

	// position
	meshVao->enableArrayAttrib(0);
	meshVao->arrayAttribFormatF(0, 4, 0, 0);
	meshVao->arrayAttribBinding(0, 0);

	// normal
	meshVao->enableArrayAttrib(1);
	meshVao->arrayAttribFormatF(1, 3, 0, sizeof(f32) * 4);
	meshVao->arrayAttribBinding(1, 0);

	// tangent
	meshVao->enableArrayAttrib(2);
	meshVao->arrayAttribFormatF(2, 3, 0, sizeof(f32) * 7);
	meshVao->arrayAttribBinding(2, 0);

	// uv
	meshVao->enableArrayAttrib(3);
	meshVao->arrayAttribFormatF(3, 2, 0, sizeof(f32) * 10);
	meshVao->arrayAttribBinding(3, 0);

	createMeshDrawIdVbo();
	createMeshIbo();

	meshVao->enableArrayAttrib(4);
	meshVao->arrayAttribFormatI(4, 1, vx::gl::DataType::Unsigned_Int, 0);
	meshVao->arrayAttribBinding(4, 1);
	meshVao->arrayBindingDivisor(1, 1);

	meshVao->bindVertexBuffer(m_coldData->m_meshVbo, 0, 0, sizeof(VertexPNTUV));
	meshVao->bindVertexBuffer(m_coldData->m_meshIdVbo, 1, 0, sizeof(u32));
	meshVao->bindIndexBuffer(m_coldData->m_meshIbo);
}

void SceneRenderer::createMeshCmdBuffer()
{
	vx::gl::BufferDescription meshCmdDesc;
	meshCmdDesc.bufferType = vx::gl::BufferType::Draw_Indirect_Buffer;
	meshCmdDesc.immutable = 1;
	meshCmdDesc.flags = vx::gl::BufferStorageFlags::Write;

	meshCmdDesc.size = sizeof(vx::gl::DrawElementsIndirectCommand) * (s_meshMaxInstancesDynamic + s_meshMaxInstancesStatic);
	m_pObjectManager->createBuffer("meshCmdBuffer", meshCmdDesc);

	u32 count = 0;
	meshCmdDesc.flags = vx::gl::BufferStorageFlags::Write | vx::gl::BufferStorageFlags::Dynamic_Storage;
	meshCmdDesc.bufferType = vx::gl::BufferType::Parameter_Buffer;
	meshCmdDesc.size = sizeof(u32) * 2;
	meshCmdDesc.pData = &count;
	m_pObjectManager->createBuffer("meshParamBuffer", meshCmdDesc);
}

void SceneRenderer::createMeshTransformBuffer()
{
	vx::gl::BufferDescription desc;
	desc.bufferType = vx::gl::BufferType::Shader_Storage_Buffer;
	desc.immutable = 1;
	desc.flags = vx::gl::BufferStorageFlags::Write | vx::gl::BufferStorageFlags::Dynamic_Storage;

	desc.size = sizeof(vx::TransformGpu) * (s_meshMaxInstancesStatic + s_meshMaxInstancesDynamic);
	m_pObjectManager->createBuffer("transformBuffer",  desc);
}

void SceneRenderer::createMeshMaterialBuffer()
{
	vx::gl::BufferDescription materialDesc;
	materialDesc.bufferType = vx::gl::BufferType::Shader_Storage_Buffer;
	materialDesc.size = sizeof(MaterialGPU) * s_maxMaterials;
	materialDesc.immutable = 1;
	materialDesc.flags = vx::gl::BufferStorageFlags::Write;
	m_pObjectManager->createBuffer("materialBlockBuffer", materialDesc);
}

void SceneRenderer::initialize(u32 maxLightCount, gl::ObjectManager* objectManager, vx::StackAllocator *pAllocator)
{
	m_pObjectManager = objectManager;
	m_coldData = vx::make_unique<ColdData>();
	m_coldData->m_scratchAllocator = vx::StackAllocator(pAllocator->allocate(1 MBYTE, 64), 1 MBYTE);

	createTextures();

	createMeshBuffers();
	createMeshCmdBuffer();
	createMeshTransformBuffer();
	createMeshMaterialBuffer();

	m_coldData->m_lightBufferManager.initialize(maxLightCount, objectManager);
}

bool SceneRenderer::initializeProfiler(const Font &font, u64 fontTextureHandle, const vx::uint2 &resolution, const vx::gl::ShaderManager &shaderManager, GpuProfiler* gpuProfiler, vx::StackAllocator *pAllocator)
{
	auto textureIndex = *m_coldData->m_texturesGPU.find(fontTextureHandle);
	if (!gpuProfiler->initialize(&font, shaderManager.getPipeline("text.pipe"), textureIndex, resolution, pAllocator))
		return false;

	CpuProfiler::initializeRenderer(shaderManager.getPipeline("text.pipe"), textureIndex, resolution);

	return true;
}

void SceneRenderer::createMaterial(Material* pMaterial)
{
	assert(pMaterial);

	auto fileAspect = Locator::getFileAspect();
	VX_ASSERT(fileAspect);

	auto pAlbedoTex = fileAspect->getTextureFile(pMaterial->m_textureSid[0]);
	VX_ASSERT(pAlbedoTex);

	auto albedoRef = m_coldData->m_textureManager.load(*pAlbedoTex, 1, 1);
	VX_ASSERT(albedoRef.isValid());

	auto pNormalTex = fileAspect->getTextureFile(pMaterial->m_textureSid[1]);
	VX_ASSERT(pNormalTex);
	VX_ASSERT(pNormalTex->getChannels() == 3);

	auto normalRef = m_coldData->m_textureManager.load(*pNormalTex, 1, 0);
	VX_ASSERT(normalRef.isValid());

	auto pSurfaceTex = fileAspect->getTextureFile(pMaterial->m_textureSid[2]);
	VX_ASSERT(pSurfaceTex);

	auto surfaceRef = m_coldData->m_textureManager.load(*pSurfaceTex, 1, 1);
	VX_ASSERT(surfaceRef.isValid());

	pMaterial->setTextures(std::move(albedoRef), std::move(normalRef), std::move(surfaceRef));
}

u16 SceneRenderer::addActorToBuffer(const vx::Transform &transform, const vx::StringID &meshSid, const vx::StringID &material, const Scene* pScene)
{
	auto itMesh = m_coldData->m_meshEntries.find(meshSid);
	VX_ASSERT(itMesh != m_coldData->m_meshEntries.end());
	if (itMesh == m_coldData->m_meshEntries.end())
	{
		auto &sceneMeshes = pScene->getMeshes();
		auto itSceneMesh = sceneMeshes.find(meshSid);

		u32 vertexOffset = m_coldData->m_dynamicMeshVertexCount + s_maxVerticesStatic;
		u32 indexOffset = m_coldData->m_dynamicMeshIndexCount + s_maxIndicesStatic;

		auto meshFile = (*itSceneMesh);

		WriteMeshesToGpuBufferDesc desc;
		desc.count = 1;
		desc.ibo = &m_coldData->m_meshIbo;
		desc.indexOffsetGpu = &indexOffset;
		desc.meshSid = &meshSid;
		desc.pMeshFiles = &meshFile;
		desc.vbo = &m_coldData->m_meshVbo;
		desc.vertexOffsetGpu = &vertexOffset;
		desc.meshEntries = &m_coldData->m_meshEntries;

		writeMeshesToGpuBuffer(desc);

		auto &mesh = meshFile->getMesh();

		m_coldData->m_dynamicMeshVertexCount += mesh.getVertexCount();
		m_coldData->m_dynamicMeshIndexCount += mesh.getIndexCount();

		itMesh = m_coldData->m_meshEntries.find(meshSid);
	}

	auto fileAspect = Locator::getFileAspect();
	auto pMaterial = fileAspect->getMaterial(material);

	// add material
	auto itMaterial = m_coldData->m_materialIndices.find(pMaterial);
	if (itMaterial == m_coldData->m_materialIndices.end())
	{
		createMaterial(pMaterial);

		writeMaterialToBuffer(pMaterial, m_coldData->m_materialCount);

		itMaterial = m_coldData->m_materialIndices.insert(pMaterial, m_coldData->m_materialCount);

		++m_coldData->m_materialCount;
	}

	// create transform and draw command
	auto materialIndex = *itMaterial;

	u32 elementId = m_dynamicMeshInstanceCount;
	u16 index = m_dynamicMeshInstanceCount;

	updateTransform(transform, elementId);
	writeMeshInstanceIdBuffer(elementId, materialIndex);

	auto cmdBuffer = m_pObjectManager->getBuffer("meshCmdBuffer");
	writeMeshInstanceToCommandBuffer(*itMesh, index, elementId, nullptr, cmdBuffer);

	++m_dynamicMeshInstanceCount;

	return elementId;
}

void SceneRenderer::updateTransform(const vx::Transform &transform, u32 elementId)
{
	auto qRotation = vx::loadFloat(transform.m_rotation);
	qRotation = vx::quaternionRotationRollPitchYawFromVector(qRotation);
	auto packedRotation = GpuFunctions::packQRotation(qRotation);

	vx::TransformGpu t;
	t.translation = transform.m_translation;
	t.scaling = transform.m_scaling;
	t.packedQRotation = packedRotation;

	updateTransform(t, elementId);
}

void SceneRenderer::updateTransform(const vx::TransformGpu &transform, u32 elementId)
{
	//auto offset = sizeof(vx::TransformGpu) * elementId;
	//m_coldData->m_transformBlock.subData(offset, sizeof(vx::TransformGpu), &transform);

	auto transformBlockBuffer = m_pObjectManager->getBuffer("transformBuffer");

	auto pTransforms = transformBlockBuffer->map<vx::TransformGpu>(vx::gl::Map::Write_Only);
	vx::memcpy(pTransforms.get() + elementId, transform);
	pTransforms.unmap();
}

void SceneRenderer::updateLights(const Light* lights, u32 count)
{
	m_coldData->m_lightBufferManager.updateLightDataBuffer(lights, count, m_pObjectManager);
}

void SceneRenderer::writeMaterialToBuffer(const Material *pMaterial, u32 offset)
{
	auto getTextureGpuEntry = [&](const TextureRef &ref)
	{
		auto handle = m_coldData->m_textureManager.getTextureHandle(ref);
		auto it = m_coldData->m_texturesGPU.find(handle);

		auto slice = ref.getSlice();
		auto index = *it;

		u32 entry = index | (slice << 16);

		return entry;
	};

	auto &albedoRef = pMaterial->getAlbedoRef();
	auto &normalRef = pMaterial->getNormalRef();
	auto &surfaceRef = pMaterial->getSurfaceRef();

	u32 hasNormalmap = (normalRef.isValid());

	auto materialBlockBuffer = m_pObjectManager->getBuffer("materialBlockBuffer");
	auto pMaterialGPU = materialBlockBuffer->mapRange<MaterialGPU>(sizeof(MaterialGPU) * offset, sizeof(MaterialGPU), vx::gl::MapRange::Write);
	VX_ASSERT(pMaterialGPU.get());
	pMaterialGPU->indexAlbedo = getTextureGpuEntry(albedoRef);

	if (hasNormalmap != 0)
		pMaterialGPU->indexNormal = getTextureGpuEntry(normalRef);

	pMaterialGPU->indexSurface = getTextureGpuEntry(surfaceRef);
	pMaterialGPU->hasNormalMap = hasNormalmap;
	pMaterialGPU.unmap();
}

void SceneRenderer::writeMeshToBuffer(const WriteMeshToBufferDesc &desc)
{
	auto &mesh = desc.pMeshFile->getMesh();
	auto pMeshVertices = mesh.getVertices();
	auto vertexCount = mesh.getVertexCount();

	//////////////////////////////
	const __m128 rotation = { vx::VX_PI, vx::VX_PI, vx::VX_PI, 0 };

	for (auto j = 0u; j < vertexCount; ++j)
	{
		VertexPNTUV vertex;
		vertex.position.x = pMeshVertices[j].position.x;
		vertex.position.y = pMeshVertices[j].position.y;
		vertex.position.z = pMeshVertices[j].position.z;
		vertex.normal = pMeshVertices[j].normal;
		vertex.tangent = pMeshVertices[j].tangent;
		vertex.uv = pMeshVertices[j].texCoords;

		f32 w = vx::dot(vx::cross(vertex.normal, vertex.tangent), pMeshVertices[j].bitangent);
		if (w < 0.0f)
		{
			vertex.position.w = -1.0f;
		}

		desc.pVertices[j + *desc.vertexOffset] = vertex;
	}

	//////////////////////////////

	auto meshIndices = mesh.getIndices();
	auto indexCount = mesh.getIndexCount();

	for (auto j = 0u; j < indexCount; ++j)
	{
		desc.pIndices[j + *desc.indexOffset] = meshIndices[j] + *desc.vertexOffsetGpu;
	}

	//////////////////////////////

	MeshEntry entry;
	entry.firstIndex = *desc.indexOffsetGpu;
	entry.indexCount = indexCount;
	desc.meshEntries->insert(desc.meshSid, entry);

	//////////////////////////////
	*desc.vertexOffsetGpu += vertexCount;
	*desc.vertexOffset += vertexCount;
	*desc.indexOffsetGpu += indexCount;
	*desc.indexOffset += indexCount;
}

void SceneRenderer::writeMeshesToGpuBuffer(const WriteMeshesToGpuBufferDesc &desc)
{
	// get total vertex and indexcount
	u32 vertexCount = 0, indexCount = 0;
	for (u32 i = 0; i < desc.count; ++i)
	{
		auto &mesh = desc.pMeshFiles[i]->getMesh();
		vertexCount += mesh.getVertexCount();
		indexCount += mesh.getIndexCount();
	}

	u32 offsetBytes = *desc.vertexOffsetGpu * sizeof(VertexPNTUV);
	u32 offsetIndicesBytes = sizeof(u32) * (*desc.indexOffsetGpu);

	auto marker = m_coldData->m_scratchAllocator.getMarker();
	SCOPE_EXIT
	{
		m_coldData->m_scratchAllocator.clear(marker);
	};

	auto vertexSizeBytes = sizeof(VertexPNTUV) * vertexCount;
	auto indexSizeBytes = sizeof(u32) * indexCount;
	VertexPNTUV* pVertices = (VertexPNTUV*)m_coldData->m_scratchAllocator.allocate(vertexSizeBytes);
	u32* pIndices = (u32*)m_coldData->m_scratchAllocator.allocate(indexSizeBytes);
	VX_ASSERT(pVertices && pIndices);

	u32 vertexOffset = 0, indexOffset = 0;
	for (u32 i = 0; i < desc.count; ++i)
	{
		WriteMeshToBufferDesc writeDesc;
		writeDesc.indexOffset = &indexOffset;
		writeDesc.indexOffsetGpu = desc.indexOffsetGpu;
		writeDesc.meshEntries = desc.meshEntries;
		writeDesc.meshSid = desc.meshSid[i];
		writeDesc.pIndices = pIndices;
		writeDesc.pMeshFile = desc.pMeshFiles[i];
		writeDesc.pVertices = pVertices;
		writeDesc.vertexOffset = &vertexOffset;
		writeDesc.vertexOffsetGpu = desc.vertexOffsetGpu;

		writeMeshToBuffer(writeDesc);
	}

	// upload to gpu

	auto pGpuVertices = desc.vbo->mapRange<VertexPNTUV>(offsetBytes, vertexSizeBytes, vx::gl::MapRange::Write);
	memcpy(pGpuVertices.get(), pVertices, vertexSizeBytes);

	auto pGpuIndices = desc.ibo->mapRange<u32>(offsetIndicesBytes, indexSizeBytes, vx::gl::MapRange::Write);
	memcpy(pGpuIndices.get(), pIndices, indexSizeBytes);
}

void SceneRenderer::updateStaticMeshBuffer(const vx::sorted_vector<vx::StringID, const vx::MeshFile*> &meshes)
{
	u32 totalVertexCount = 0;
	u32 totalIndexCount = 0;
	auto meshCount = meshes.size();

	WriteMeshesToGpuBufferDesc desc;
	desc.count = meshCount;
	desc.ibo = &m_coldData->m_meshIbo;
	desc.indexOffsetGpu = &totalIndexCount;
	desc.meshSid = meshes.keys();
	desc.pMeshFiles = meshes.data();
	desc.vbo = &m_coldData->m_meshVbo;
	desc.vertexOffsetGpu = &totalVertexCount;
	desc.meshEntries = &m_coldData->m_meshEntries;

	writeMeshesToGpuBuffer(desc);
}

void SceneRenderer::updateLightBuffer(const Light *pLights, u32 lightCount, const gl::ObjectManager &objectManager)
{
	assert(lightCount <= 5);

	ShadowTransformBlock shadowTransforms;

	for (auto i = 0u; i < lightCount; ++i)
	{
		pLights[i].getShadowTransform(&shadowTransforms.transforms[i]);
	}

	m_coldData->m_lightBufferManager.updateLightDataBuffer(pLights, lightCount, m_pObjectManager);

	auto pShadowTransformBuffer = objectManager.getBuffer("ShadowTransformBuffer");
	auto shadowTransformsMappedBuffer = pShadowTransformBuffer->map<ShadowTransformBlock>(vx::gl::Map::Write_Only);
	vx::memcpy(shadowTransformsMappedBuffer.get(), shadowTransforms);
}

void SceneRenderer::writeMeshInstanceIdBuffer(u32 elementId, u32 materialIndex)
{
	auto drawId = elementId | (materialIndex << 16);
	auto pMeshId = m_coldData->m_meshIdVbo.map<u32>(vx::gl::Map::Write_Only);
	pMeshId[elementId] = drawId;
}

void SceneRenderer::writeMeshInstanceToCommandBuffer(MeshEntry meshEntry, u32 index, u32 elementId, vx::gl::DrawElementsIndirectCommand* drawCmd, const vx::gl::Buffer* cmdBuffer)
{
	vx::gl::DrawElementsIndirectCommand cmd;
	cmd.count = meshEntry.indexCount;
	cmd.instanceCount = 1;
	cmd.firstIndex = meshEntry.firstIndex;
	cmd.baseVertex = 0;
	cmd.baseInstance = elementId;

	if (drawCmd)
		*drawCmd = cmd;

	auto mappedCmdBuffer = cmdBuffer->map<vx::gl::DrawElementsIndirectCommand>(vx::gl::Map::Write_Only);
	mappedCmdBuffer[index] = cmd;
}

void addMeshToGpu(u32* vertexOffset, u32* indexOffset)
{

}

void SceneRenderer::updateBuffers(const MeshInstance *pInstances, u32 instanceCount, const vx::sorted_vector<const Material*, u32> &materialIndices, const vx::sorted_vector<vx::StringID, MeshEntry> &meshEntries)
{
	if (instanceCount == 0)
	{
		return;
	}

	u32 totalInstanceCount = 0;
	u32 drawCount = 0;

	auto pFileAspect = Locator::getFileAspect();

	auto cmdBuffer = m_pObjectManager->getBuffer("meshCmdBuffer");

	for (auto i = 0u; i < instanceCount; ++i)
	{
		auto currentMeshSid = pInstances[i].getMeshSid();
		auto meshEntry = meshEntries.find(currentMeshSid);

		auto pCurrentMaterial = pFileAspect->getMaterial(pInstances[i].getMaterialSid());
		auto materialIndex = *materialIndices.find(pCurrentMaterial);

		u16 elementId = i;

		auto transform = pInstances[i].getTransform();
		transform.m_rotation = vx::degToRad(transform.m_rotation);

		updateTransform(transform, elementId);
		writeMeshInstanceIdBuffer(elementId, materialIndex);

		vx::gl::DrawElementsIndirectCommand cmd;
		writeMeshInstanceToCommandBuffer(*meshEntry, elementId, elementId, &cmd, cmdBuffer);

		m_coldData->m_instanceCmds.insert(&pInstances[i], cmd);

		++totalInstanceCount;
	}

	auto meshParamBuffer = m_pObjectManager->getBuffer("meshParamBuffer");
	auto mappedParamBuffer = meshParamBuffer->map<u32>(vx::gl::Map::Write_Only);
	*mappedParamBuffer = instanceCount;
}

void SceneRenderer::loadScene(const Scene &scene, const gl::ObjectManager &objectManager)
{
	auto &sceneMaterial = scene.getMaterials();
	auto numMaterials = scene.getMaterialCount();

	assert(m_coldData->m_materialCount + numMaterials <= s_maxMaterials);

	auto materialOffset = 0u;
	m_coldData->m_materialIndices.reserve(numMaterials);
	for (auto i = 0u; i < numMaterials; ++i)
	{
		auto pMaterial = sceneMaterial[i];
		createMaterial(pMaterial);

		writeMaterialToBuffer(pMaterial, m_coldData->m_materialCount);

		m_coldData->m_materialIndices.insert(pMaterial, m_coldData->m_materialCount);
		++m_coldData->m_materialCount;
	}

	m_coldData->m_meshEntries.reserve(scene.getMeshes().size());

	updateLightBuffer(scene.getLights(), scene.getLightCount(), objectManager);
	updateStaticMeshBuffer(scene.getMeshes());
	updateBuffers(scene.getMeshInstances(), scene.getMeshInstanceCount(), m_coldData->m_materialIndices, m_coldData->m_meshEntries);

	m_staticMeshInstanceCount = scene.getMeshInstanceCount();

	m_meshInstanceCount = m_staticMeshInstanceCount + m_dynamicMeshInstanceCount;
}

TextureRef SceneRenderer::loadTexture(const char* file)
{
	TextureRef ref;

	TextureFile texture;
	if (texture.loadFromFile(file))
	{
		ref = m_coldData->m_textureManager.load(texture, 1, 1);
	}

	return ref;
}

u16 SceneRenderer::getActorGpuIndex()
{
	u32 elementId = s_meshMaxInstancesStatic + m_dynamicMeshInstanceCount++;

	VX_ASSERT(u16(elementId) == elementId);

	return elementId;
}

vx::gl::DrawElementsIndirectCommand SceneRenderer::getDrawCommand(const MeshInstance* p) const
{
	vx::gl::DrawElementsIndirectCommand cmd;
	cmd.count = 0;

	auto it = m_coldData->m_instanceCmds.find(p);
	if (it != m_coldData->m_instanceCmds.end())
	{
		cmd = *it;
	}

	return cmd;
}

u32 SceneRenderer::getLightCount() const
{
	return m_coldData->m_lightBufferManager.getLightCount();
}