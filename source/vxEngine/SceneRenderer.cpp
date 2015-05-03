#include "SceneRenderer.h"
#include "Scene.h"
#include "Vertex.h"
#include "GpuStructs.h"
#include "Transform.h"
#include "GpuFunctions.h"
#include "utility.h"
#include "Locator.h"
#include "FileAspect.h"
#include <vxLib/ScopeGuard.h>
#include "MeshInstance.h"
#include "BufferManager.h"
#include "BufferBindingManager.h"
#include "Light.h"
#include <vxLib/gl/StateManager.h>
#include <vxLib/gl/gl.h>
#include "TextureFile.h"
#include "GpuProfiler.h"
#include <vxLib/gl/ShaderManager.h>

SceneRenderer::SceneRenderer()
{

}

SceneRenderer::~SceneRenderer()
{

}

void SceneRenderer::createTextures(BufferManager* pBufferManager)
{
	m_coldData->m_textureManager.createBucket(1, vx::ushort3(1024, 1024, 10), 1, vx::gl::TextureType::Texture_2D_Array, vx::gl::TextureFormat::SRGBA8);
	m_coldData->m_textureManager.createBucket(1, vx::ushort3(1024, 1024, 10), 1, vx::gl::TextureType::Texture_2D_Array, vx::gl::TextureFormat::SRGB8);
	m_coldData->m_textureManager.createBucket(1, vx::ushort3(1024, 1024, 10), 1, vx::gl::TextureType::Texture_2D_Array, vx::gl::TextureFormat::RGB8);

	const auto numHandles = 3u;
	U64 handles[numHandles];
	handles[0] = m_coldData->m_textureManager.createTexture(vx::ushort3(1024, 1024, 10), 1, vx::gl::TextureType::Texture_2D_Array, vx::gl::TextureFormat::SRGBA8);
	handles[1] = m_coldData->m_textureManager.createTexture(vx::ushort3(1024, 1024, 10), 1, vx::gl::TextureType::Texture_2D_Array, vx::gl::TextureFormat::RGB8);
	handles[2] = m_coldData->m_textureManager.createTexture(vx::ushort3(1024, 1024, 10), 1, vx::gl::TextureType::Texture_2D_Array, vx::gl::TextureFormat::SRGB8);

	for (U32 i = 0; i < numHandles; ++i)
	{
		m_coldData->m_texturesGPU.insert(handles[i], i);
	}

	vx::gl::BufferDescription desc;
	desc.bufferType = vx::gl::BufferType::Shader_Storage_Buffer;
	desc.size = sizeof(U64) * numHandles;
	desc.immutable = 1;
	desc.pData = handles;

	pBufferManager->createBuffer("TextureBuffer", desc);
}

void SceneRenderer::createMeshDrawIdVbo()
{
	vx::gl::BufferDescription idVboDesc;
	idVboDesc.bufferType = vx::gl::BufferType::Array_Buffer;
	idVboDesc.size = sizeof(U32) * s_meshMaxInstances;
	idVboDesc.flags = vx::gl::BufferStorageFlags::Write;
	idVboDesc.immutable = 1;
	m_coldData->m_meshIdVbo.create(idVboDesc);
}

void SceneRenderer::createMeshIbo()
{
	vx::gl::BufferDescription iboDesc;
	iboDesc.bufferType = vx::gl::BufferType::Element_Array_Buffer;
	iboDesc.size = sizeof(U32) * s_maxIndicesTotal;
	iboDesc.flags = vx::gl::BufferStorageFlags::Write;
	iboDesc.immutable = 1;
	m_coldData->m_meshIbo.create(iboDesc);
}

void SceneRenderer::bindMeshDrawIdVboToVao(vx::gl::VertexArray* vao)
{
	vao->enableArrayAttrib(4);
	vao->arrayAttribFormatI(4, 1, vx::gl::DataType::Unsigned_Int, 0);
	vao->arrayAttribBinding(4, 1);
	vao->arrayBindingDivisor(1, 1);
	vao->bindVertexBuffer(m_coldData->m_meshIdVbo, 1, 0, sizeof(U32));
}

void SceneRenderer::createMeshBuffers()
{
	U32 attributeOffset = 0;
	VertexPNTUV::create(&m_meshVao, &m_coldData->m_meshVbo, s_maxVerticesTotal, 0, attributeOffset);

	createMeshDrawIdVbo();
	createMeshIbo();

	bindMeshDrawIdVboToVao(&m_meshVao);
	m_meshVao.bindIndexBuffer(m_coldData->m_meshIbo);
}

void SceneRenderer::createMeshCmdBuffer()
{
	vx::gl::BufferDescription meshCmdDesc;
	meshCmdDesc.bufferType = vx::gl::BufferType::Draw_Indirect_Buffer;
	meshCmdDesc.size = sizeof(vx::gl::DrawElementsIndirectCommand) * s_meshMaxInstances;
	meshCmdDesc.immutable = 1;
	meshCmdDesc.flags = vx::gl::BufferStorageFlags::Write;

	m_commandBlock.create(meshCmdDesc);
}

void SceneRenderer::createMeshTransformBuffer()
{
	vx::gl::BufferDescription desc;
	desc.bufferType = vx::gl::BufferType::Shader_Storage_Buffer;
	desc.size = sizeof(vx::TransformGpu) * s_meshMaxInstances;
	desc.immutable = 1;
	desc.flags = vx::gl::BufferStorageFlags::Write | vx::gl::BufferStorageFlags::Dynamic_Storage;
	m_coldData->m_transformBlock.create(desc);
}

void SceneRenderer::createMeshMaterialBuffer()
{
	vx::gl::BufferDescription materialDesc;
	materialDesc.bufferType = vx::gl::BufferType::Shader_Storage_Buffer;
	materialDesc.size = sizeof(MaterialGPU) * s_maxMaterials;
	materialDesc.immutable = 1;
	materialDesc.flags = vx::gl::BufferStorageFlags::Write;
	m_coldData->m_materialBlock.create(materialDesc);
}

void SceneRenderer::initialize(U32 maxLightCount, BufferManager* pBufferManager, vx::StackAllocator *pAllocator)
{
	m_coldData = std::make_unique<ColdData>();
	m_coldData->m_scratchAllocator = vx::StackAllocator(pAllocator->allocate(1 MBYTE, 64), 1 MBYTE);

	createTextures(pBufferManager);

	createMeshBuffers();
	createMeshCmdBuffer();
	createMeshTransformBuffer();
	createMeshMaterialBuffer();

	m_coldData->m_lightBufferManager.initialize(maxLightCount);
}

bool SceneRenderer::initializeProfiler(const Font &font, U64 fontTextureHandle, const vx::uint2 &resolution, const vx::gl::ShaderManager &shaderManager, GpuProfiler* gpuProfiler, vx::StackAllocator *pAllocator)
{
	auto textureIndex = *m_coldData->m_texturesGPU.find(fontTextureHandle);
	if (!gpuProfiler->initialize(&font, shaderManager.getPipeline("text.pipe"), textureIndex, resolution, pAllocator))
		return false;

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

U16 SceneRenderer::addActorToBuffer(const vx::Transform &transform, const vx::StringID &mesh, const vx::StringID &material, const Scene* pScene)
{
	auto itMesh = m_coldData->m_meshEntries.find(mesh);
	if (itMesh == m_coldData->m_meshEntries.end())
	{
		auto &sceneMeshes = pScene->getMeshes();
		auto itSceneMesh = sceneMeshes.find(mesh);

		U32 vertexOffset = m_coldData->m_meshVertexCountDynamic + s_maxVerticesStatic;
		U32 indexOffset = m_coldData->m_meshIndexCountDynamic + s_maxIndicesStatic;

		writeMeshesToVertexBuffer(&mesh, &(*itSceneMesh), 1, &vertexOffset, &indexOffset);
		//writeMeshToVertexBuffer(mesh, (*itSceneMesh), );

		m_coldData->m_meshVertexCountDynamic += (*itSceneMesh)->getVertexCount();
		m_coldData->m_meshIndexCountDynamic += (*itSceneMesh)->getIndexCount();

		itMesh = m_coldData->m_meshEntries.find(mesh);
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

	U32 elementId = s_meshMaxInstancesStatic + m_coldData->m_meshInstanceCountDynamic;
	U16 index = m_meshInstancesCountTotal;

	updateTransform(transform, elementId);
	writeMeshInstanceIdBuffer(elementId, materialIndex);

	writeMeshInstanceToCommandBuffer(*itMesh, index, elementId, nullptr);

	++m_coldData->m_meshInstanceCountDynamic;

	m_meshInstancesCountTotal = m_coldData->m_meshInstanceCountStatic + m_coldData->m_meshInstanceCountDynamic;

	return elementId;
}

void SceneRenderer::updateTransform(const vx::Transform &transform, U32 elementId)
{
	auto qRotation = vx::loadFloat(transform.m_rotation);
	qRotation = vx::QuaternionRotationRollPitchYawFromVector(qRotation);
	auto packedRotation = GpuFunctions::packQRotation(qRotation);

	vx::TransformGpu t;
	t.translation = transform.m_translation;
	t.scaling = transform.m_scaling;
	t.packedQRotation = packedRotation;

	updateTransform(t, elementId);
}

void SceneRenderer::updateTransform(const vx::TransformGpu &transform, U32 elementId)
{
	auto pTransforms = m_coldData->m_transformBlock.map<vx::TransformGpu>(vx::gl::Map::Write_Only);
	vx::memcpy(pTransforms.get() + elementId, transform);
	pTransforms.unmap();
}

void SceneRenderer::writeMaterialToBuffer(const Material *pMaterial, U32 offset)
{
	auto getTextureGpuEntry = [&](const TextureRef &ref)
	{
		auto handle = m_coldData->m_textureManager.getTextureHandle(ref);
		auto it = m_coldData->m_texturesGPU.find(handle);

		auto slice = ref.getSlice();
		auto index = *it;

		U32 entry = index | (slice << 16);

		return entry;
	};

	auto &albedoRef = pMaterial->getAlbedoRef();
	auto &normalRef = pMaterial->getNormalRef();
	auto &surfaceRef = pMaterial->getSurfaceRef();

	U32 hasNormalmap = (normalRef.isValid());

	auto pMaterialGPU = m_coldData->m_materialBlock.mapRange<MaterialGPU>(sizeof(MaterialGPU) * offset, sizeof(MaterialGPU), vx::gl::MapRange::Write);
	VX_ASSERT(pMaterialGPU.get());
	pMaterialGPU->indexAlbedo = getTextureGpuEntry(albedoRef);

	if (hasNormalmap != 0)
		pMaterialGPU->indexNormal = getTextureGpuEntry(normalRef);

	pMaterialGPU->indexSurface = getTextureGpuEntry(surfaceRef);
	pMaterialGPU->hasNormalMap = hasNormalmap;
	pMaterialGPU.unmap();
}

void SceneRenderer::writeMeshToBuffer(const vx::StringID &meshSid, const vx::Mesh* pMesh, VertexPNTUV* pVertices, U32* pIndices, U32* vertexOffset, U32* indexOffset, U32 *vertexOffsetGpu, U32 *indexOffsetGpu)
{
	auto pMeshVertices = pMesh->getVertices();
	auto vertexCount = pMesh->getVertexCount();

	//////////////////////////////
	const __m128 rotation = { vx::VX_PI, vx::VX_PI, vx::VX_PI, 0 };

	auto getRotationQuat = [](const __m128 &from, const __m128 &to)
	{
		__m128 H = _mm_add_ps(from, to);
		H = vx::Vector3Normalize(H);
		auto dot = vx::Vector3Dot(from, H);

		__m128 result;
		result.f[3] = dot.f[0];

		result.f[0] = from.f[1] * H.f[2] - from.f[2] * H.f[1];
		result.f[1] = from.f[2] * H.f[0] - from.f[0] * H.f[2];
		result.f[2] = from.f[0] * H.f[1] - from.f[1] * H.f[0];

		return result;
	};

	for (auto j = 0u; j < vertexCount; ++j)
	{
		VertexPNTUV vertex;
		vertex.position.x = pMeshVertices[j].position.x;
		vertex.position.y = pMeshVertices[j].position.y;
		vertex.position.z = pMeshVertices[j].position.z;
		vertex.normal = pMeshVertices[j].normal;
		vertex.tangent = pMeshVertices[j].tangent;
		vertex.uv = pMeshVertices[j].texCoords;

		/*vx::mat4 tbnMatrix;
		tbnMatrix.c[0] = vx::loadFloat(pMeshVertices[j].tangent);
		tbnMatrix.c[1] = vx::loadFloat(pMeshVertices[j].bitangent);
		tbnMatrix.c[2] = vx::loadFloat(pMeshVertices[j].normal);

		auto qtbn = Quaternion::create(tbnMatrix);

		auto vnormal = vx::loadFloat(vertex.normal);

		auto nn = vx::Vector3Rotate(vnormal, qtbn.v);*/

		F32 w = vx::dot(vx::cross(vertex.normal, vertex.tangent), pMeshVertices[j].bitangent);
		if (w < 0.0f)
		{
			vertex.position.w = -1.0f;
		}

		pVertices[j + *vertexOffset] = vertex;
	}

	//////////////////////////////

	auto meshIndices = pMesh->getIndices();
	auto indexCount = pMesh->getIndexCount();

	for (auto j = 0u; j < indexCount; ++j)
	{
		pIndices[j + *indexOffset] = meshIndices[j] + *vertexOffsetGpu;
	}

	//////////////////////////////

	MeshEntry entry;
	entry.firstIndex = *indexOffsetGpu;
	entry.indexCount = indexCount;
	m_coldData->m_meshEntries.insert(meshSid, entry);

	//////////////////////////////
	*vertexOffsetGpu += vertexCount;
	*vertexOffset += vertexCount;
	*indexOffsetGpu += indexCount;
	*indexOffset += indexCount;
}

void SceneRenderer::writeMeshesToVertexBuffer(const vx::StringID* meshSid, const vx::Mesh** pMesh, U32 count, U32 *vertexOffsetGpu, U32 *indexOffsetGpu)
{
	// get total vertex and indexcount
	U32 vertexCount = 0, indexCount = 0;
	for (U32 i = 0; i < count; ++i)
	{
		vertexCount += pMesh[i]->getVertexCount();
		indexCount += pMesh[i]->getIndexCount();
	}

	U32 offsetBytes = *vertexOffsetGpu * sizeof(VertexPNTUV);
	U32 offsetIndicesBytes = sizeof(U32) * (*indexOffsetGpu);

	auto marker = m_coldData->m_scratchAllocator.getMarker();
	SCOPE_EXIT
	{
		m_coldData->m_scratchAllocator.clear(marker);
	};

	auto vertexSizeBytes = sizeof(VertexPNTUV) * vertexCount;
	auto indexSizeBytes = sizeof(U32) * indexCount;
	VertexPNTUV* pVertices = (VertexPNTUV*)m_coldData->m_scratchAllocator.allocate(vertexSizeBytes);
	U32* pIndices = (U32*)m_coldData->m_scratchAllocator.allocate(indexSizeBytes);
	VX_ASSERT(pVertices && pIndices);

	U32 vertexOffset = 0, indexOffset = 0;
	for (U32 i = 0; i < count; ++i)
	{
		writeMeshToBuffer(meshSid[i], pMesh[i], pVertices, pIndices, &vertexOffset, &indexOffset, vertexOffsetGpu, indexOffsetGpu);
	}

	// upload to gpu

	auto pGpuVertices = m_coldData->m_meshVbo.mapRange<VertexPNTUV>(offsetBytes, vertexSizeBytes, vx::gl::MapRange::Write);
	memcpy(pGpuVertices.get(), pVertices, vertexSizeBytes);

	auto pGpuIndices = m_coldData->m_meshIbo.mapRange<U32>(offsetIndicesBytes, indexSizeBytes, vx::gl::MapRange::Write);
	memcpy(pGpuIndices.get(), pIndices, indexSizeBytes);
}

void SceneRenderer::updateMeshBuffer(const vx::sorted_vector<vx::StringID, const vx::Mesh*> &meshes)
{
	U32 totalVertexCount = 0;
	U32 totalIndexCount = 0;
	auto meshCount = meshes.size();
	writeMeshesToVertexBuffer(meshes.keys(), meshes.data(), meshCount, &totalVertexCount, &totalIndexCount);
}

void SceneRenderer::updateLightBuffer(const Light *pLights, U32 lightCount, const BufferManager &bufferManager)
{
	assert(lightCount <= 5);

	ShadowTransformBlock shadowTransforms;

	for (auto i = 0u; i < lightCount; ++i)
	{
		pLights[i].getShadowTransform(&shadowTransforms.transforms[i]);
	}

	m_coldData->m_lightBufferManager.updateLightDataBuffer(pLights, lightCount);

	auto pShadowTransformBuffer = bufferManager.getBuffer("ShadowTransformBuffer");
	auto shadowTransformsMappedBuffer = pShadowTransformBuffer->map<ShadowTransformBlock>(vx::gl::Map::Write_Only);
	vx::memcpy(shadowTransformsMappedBuffer.get(), shadowTransforms);
}

void SceneRenderer::writeMeshInstanceIdBuffer(U32 elementId, U32 materialIndex)
{
	auto drawId = elementId | (materialIndex << 16);
	auto pMeshId = m_coldData->m_meshIdVbo.map<U32>(vx::gl::Map::Write_Only);
	pMeshId[elementId] = drawId;
}

void SceneRenderer::writeMeshInstanceToCommandBuffer(MeshEntry meshEntry, U32 index, U32 elementId, vx::gl::DrawElementsIndirectCommand* drawCmd)
{
	vx::gl::DrawElementsIndirectCommand cmd;
	cmd.count = meshEntry.indexCount;
	cmd.instanceCount = 1;
	cmd.firstIndex = meshEntry.firstIndex;
	cmd.baseVertex = 0;
	cmd.baseInstance = elementId;

	if (drawCmd)
		*drawCmd = cmd;

	auto mappedCmdBuffer = m_commandBlock.map<vx::gl::DrawElementsIndirectCommand>(vx::gl::Map::Write_Only);
	mappedCmdBuffer[index] = cmd;
}

void SceneRenderer::updateBuffers(const MeshInstance *pInstances, U32 instanceCount, const vx::sorted_vector<const Material*, U32> &materialIndices, const vx::sorted_vector<vx::StringID, MeshEntry> &meshEntries)
{
	if (instanceCount == 0)
	{
		return;
	}

	U32 totalInstanceCount = 0;
	U32 drawCount = 0;

	U32 batchIndexCount = 0;
	U32 batchIndexStart = 0;
	U32 batchInstanceCount = 0;
	U32 batchInstanceStart = 0;

	auto batchMeshSid = pInstances[0].getMeshSid();
	batchIndexCount = meshEntries.find(batchMeshSid)->indexCount;

	auto pFileAspect = Locator::getFileAspect();

	for (auto i = 0u; i < instanceCount; ++i)
	{
		auto currentMeshSid = pInstances[i].getMeshSid();
		auto meshEntry = meshEntries.find(currentMeshSid);
		auto pCurrentMaterial = pFileAspect->getMaterial(pInstances[i].getMaterialSid());
		auto materialIndex = *materialIndices.find(pCurrentMaterial);

		U16 elementId = i;

		auto transform = pInstances[i].getTransform();
		transform.m_rotation = vx::degToRad(transform.m_rotation);

		updateTransform(transform, elementId);
		writeMeshInstanceIdBuffer(elementId, materialIndex);

		vx::gl::DrawElementsIndirectCommand cmd;
		writeMeshInstanceToCommandBuffer(*meshEntry, elementId, elementId, &cmd);

		m_coldData->m_instanceCmds.insert(&pInstances[i], cmd);

		++batchInstanceCount;
		++totalInstanceCount;
	}
}

void SceneRenderer::bindTransformBuffer()
{
	BufferBindingManager::bindBaseShaderStorage(0, m_coldData->m_transformBlock.getId());
}

void SceneRenderer::bindMaterialBuffer()
{
	BufferBindingManager::bindBaseShaderStorage(1, m_coldData->m_materialBlock.getId());
}

void SceneRenderer::bindBuffers()
{
	BufferBindingManager::bindBaseUniform(4, m_commandBlock.getId());

	bindTransformBuffer();
	bindMaterialBuffer();

	m_coldData->m_lightBufferManager.bindBuffer();
}

void SceneRenderer::loadScene(const Scene &scene, const BufferManager &bufferManager)
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

	updateLightBuffer(scene.getLights(), scene.getLightCount(), bufferManager);
	updateMeshBuffer(scene.getMeshes());
	updateBuffers(scene.getMeshInstances(), scene.getMeshInstanceCount(), m_coldData->m_materialIndices, m_coldData->m_meshEntries);

	m_coldData->m_meshInstanceCountStatic = scene.getMeshInstanceCount();

	m_meshInstancesCountTotal = m_coldData->m_meshInstanceCountStatic + m_coldData->m_meshInstanceCountDynamic;
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

U16 SceneRenderer::getActorGpuIndex()
{
	U32 elementId = s_meshMaxInstancesStatic + m_coldData->m_meshInstanceCountDynamic;

	VX_ASSERT(U16(elementId) == elementId);

	++m_coldData->m_meshInstanceCountDynamic;

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