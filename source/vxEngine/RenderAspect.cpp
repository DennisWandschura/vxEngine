#include "RenderAspect.h"
#include <vxLib\math\math.h>
#include <vxLib/gl/VertexArray.h>
#include <vxLib\gl\Debug.h>
#include <vxLib\gl\gl.h>
#include "UniformBlocks.h"
#include <vxLib\RawInput.h>
#include <vxLib\math\math.h>
#include "FileEntry.h"
#include "FileAspect.h"
#include "Scene.h"
#include "Light.h"
#include "Profiler2.h"
#include "Vertex.h"
#include <vxLib/stb_image_write.h>
#include <future>
#include "ProfilerGraph.h"
#include "MeshInstance.h"
#include "PhysicsAspect.h"
#include "enums.h"
#include <vxLib/ScopeGuard.h>
#include "Event.h"
#include "EventTypes.h"
#include "utility.h"
#include "NavGraph.h"
#include "NavNode.h"
#include "NavConnection.h"
#include "developer.h"
#include "EventsIngame.h"
#include "ImageBindingManager.h"
#include "BufferBindingManager.h"
#include <vxLib/gl\StateManager.h>

RenderAspect *g_renderAspect{ nullptr };

namespace
{
	void __stdcall debugCallback()
	{
		assert(false);
	}
}

RenderAspect::RenderAspect(Logfile &logfile, FileAspect &fileAspect)
	:m_shaderManager(logfile),
	m_renderContext(),
	m_camera(),
	m_fileAspect(fileAspect),
	m_pColdData()
{
	g_renderAspect = this;
}

RenderAspect::~RenderAspect()
{
}

TextureRef RenderAspect::loadTextureFile(const TextureFile &textureFile, U8 srgb)
{
	return m_pColdData->m_textureManager.load(textureFile, 1, srgb);
}

bool RenderAspect::initializeBuffers()
{
	{
		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Atomic_Counter_Buffer;
		desc.size = sizeof(U32) * 4;
		desc.pData = nullptr;
		desc.immutable = 1;
		desc.flags = vx::gl::BufferStorageFlags::Write | vx::gl::BufferStorageFlags::Read;
		m_atomicCounter.create(desc);
	}

	m_emptyVao.create();

	vx::gl::BufferDescription vboDesc;
	vboDesc.bufferType = vx::gl::BufferType::Array_Buffer;
	vboDesc.size = sizeof(VertexPNTUV) * s_maxVerticesTotal;
	vboDesc.flags = vx::gl::BufferStorageFlags::Write;
	vboDesc.immutable = 1;
	m_pColdData->m_meshVbo.create(vboDesc);

	vx::gl::BufferDescription idVboDesc;
	idVboDesc.bufferType = vx::gl::BufferType::Array_Buffer;
	idVboDesc.size = sizeof(U32) * s_meshMaxInstances;
	idVboDesc.flags = vx::gl::BufferStorageFlags::Write;
	idVboDesc.immutable = 1;
	m_pColdData->m_meshIdVbo.create(idVboDesc);

	vx::gl::BufferDescription iboDesc;
	iboDesc.bufferType = vx::gl::BufferType::Element_Array_Buffer;
	iboDesc.size = sizeof(U32) * s_maxIndicesTotal;
	iboDesc.flags = vx::gl::BufferStorageFlags::Write;
	iboDesc.immutable = 1;
	m_pColdData->m_meshIbo.create(iboDesc);

	/*
	vx::float4 position;
	vx::float3 normal;
	vx::float3 tangent;
	vx::float2 uv;
	*/

	m_meshVao.create();

	// position
	m_meshVao.enableArrayAttrib(0);
	m_meshVao.arrayAttribFormatF(0, 4, 0, 0);
	m_meshVao.arrayAttribBinding(0, 0);

	//normal
	m_meshVao.enableArrayAttrib(1);
	m_meshVao.arrayAttribFormatF(1, 3, 0, sizeof(F32) * 4);
	m_meshVao.arrayAttribBinding(1, 0);

	// tangent
	m_meshVao.enableArrayAttrib(2);
	m_meshVao.arrayAttribFormatF(2, 3, 0, sizeof(F32) * 7);
	m_meshVao.arrayAttribBinding(2, 0);

	// uv
	m_meshVao.enableArrayAttrib(3);
	m_meshVao.arrayAttribFormatF(3, 2, 0, sizeof(F32) * 10);
	m_meshVao.arrayAttribBinding(3, 0);

	m_meshVao.bindVertexBuffer(m_pColdData->m_meshVbo, 0, 0, sizeof(VertexPNTUV));

	// draw id
	m_meshVao.enableArrayAttrib(4);
	m_meshVao.arrayAttribFormatI(4, 1, vx::gl::UNSIGNED_INT, 0);
	m_meshVao.arrayAttribBinding(4, 1);
	m_meshVao.arrayBindingDivisor(1, 1);
	m_meshVao.bindVertexBuffer(m_pColdData->m_meshIdVbo, 1, 0, sizeof(U32));

	m_meshVao.bindIndexBuffer(m_pColdData->m_meshIbo);

	{
		vx::gl::BufferDescription vboDesc;
		vboDesc.bufferType = vx::gl::BufferType::Array_Buffer;
		vboDesc.size = sizeof(vx::float3) * s_maxNavMeshVertices;
		vboDesc.flags = vx::gl::BufferStorageFlags::Write;
		vboDesc.immutable = 1;
		m_pColdData->m_navmeshVbo.create(vboDesc);
	}

	{
		vx::gl::BufferDescription iboDesc;
		iboDesc.bufferType = vx::gl::BufferType::Element_Array_Buffer;
		iboDesc.size = sizeof(U16) * s_maxNavMeshIndices;
		iboDesc.flags = vx::gl::BufferStorageFlags::Write;
		iboDesc.immutable = 1;
		m_pColdData->m_navmeshIbo.create(iboDesc);
	}

	m_navmeshVao.create();
	m_navmeshVao.enableArrayAttrib(0);
	m_navmeshVao.arrayAttribFormatF(0, 3, 0, 0);
	m_navmeshVao.arrayAttribBinding(0, 0);
	m_navmeshVao.bindVertexBuffer(m_pColdData->m_navmeshVbo, 0, 0, sizeof(vx::float3));
	m_navmeshVao.bindIndexBuffer(m_pColdData->m_navmeshIbo);

	{
		vx::gl::BufferDescription vboDesc;
		vboDesc.bufferType = vx::gl::BufferType::Array_Buffer;
		vboDesc.size = sizeof(vx::float3) * s_maxNavMeshVertices;
		vboDesc.flags = vx::gl::BufferStorageFlags::Write;
		vboDesc.immutable = 1;
		m_pColdData->m_navNodesVbo.create(vboDesc);
	}

	{
		U16 indices[s_maxNavMeshIndices];
		for (U32 i = 0; i < s_maxNavMeshIndices; ++i)
		{
			indices[i] = i;
		}

		vx::gl::BufferDescription iboDesc;
		iboDesc.bufferType = vx::gl::BufferType::Element_Array_Buffer;
		iboDesc.size = sizeof(U16) * s_maxNavMeshIndices;
		iboDesc.immutable = 1;
		iboDesc.pData = indices;
		m_pColdData->m_navNodesIbo.create(iboDesc);
	}

	{
		vx::gl::BufferDescription iboDesc;
		iboDesc.bufferType = vx::gl::BufferType::Element_Array_Buffer;
		iboDesc.size = sizeof(U16) * s_maxNavMeshIndices * 2;
		iboDesc.flags = vx::gl::BufferStorageFlags::Write;
		iboDesc.immutable = 1;
		m_pColdData->m_navConnectionsIbo.create(iboDesc);
	}

	{
		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Shader_Storage_Buffer;
		desc.size = sizeof(U32);
		desc.immutable = 1;
		desc.flags = vx::gl::BufferStorageFlags::Write | vx::gl::BufferStorageFlags::Read;
		m_rayOffsetBlock.create(desc);
	}

	{
		DrawArraysIndirectCommand cmd;
		cmd.baseInstance = 0;
		cmd.count = 0;
		cmd.first = 0;
		cmd.instanceCount = 1;

		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Draw_Indirect_Buffer;
		desc.size = sizeof(DrawArraysIndirectCommand);
		desc.immutable = 1;
		desc.flags = vx::gl::BufferStorageFlags::Write | vx::gl::BufferStorageFlags::Read;
		desc.pData = &cmd;
		m_voxelTrianglePairCmdBuffer.create(desc);
	}

	m_navNodesVao.create();
	m_navNodesVao.enableArrayAttrib(0);
	m_navNodesVao.arrayAttribFormatF(0, 3, 0, 0);
	m_navNodesVao.arrayAttribBinding(0, 0);
	m_navNodesVao.bindVertexBuffer(m_pColdData->m_navNodesVbo, 0, 0, sizeof(vx::float3));
	m_navNodesVao.bindIndexBuffer(m_pColdData->m_navNodesIbo);

	m_navConnectionsVao.create();
	m_navConnectionsVao.enableArrayAttrib(0);
	m_navConnectionsVao.arrayAttribFormatF(0, 3, 0, 0);
	m_navConnectionsVao.arrayAttribBinding(0, 0);
	m_navConnectionsVao.bindVertexBuffer(m_pColdData->m_navNodesVbo, 0, 0, sizeof(vx::float3));
	m_navConnectionsVao.bindIndexBuffer(m_pColdData->m_navConnectionsIbo);

	vx::gl::BufferDescription meshCmdDesc;
	meshCmdDesc.bufferType = vx::gl::BufferType::Draw_Indirect_Buffer;
	meshCmdDesc.size = sizeof(vx::gl::DrawElementsIndirectCommand) * s_meshMaxInstances;
	meshCmdDesc.immutable = 1;
	meshCmdDesc.flags = vx::gl::BufferStorageFlags::Write;
	//m_meshIndirectBuffer.create(meshCmdDesc);

	m_commandBlock.create(meshCmdDesc);

	vx::gl::BufferDescription desc;
	desc.bufferType = vx::gl::BufferType::Shader_Storage_Buffer;
	desc.size = sizeof(vx::TransformGpu) * s_meshMaxInstances;
	desc.immutable = 1;
	desc.flags = vx::gl::BufferStorageFlags::Write | vx::gl::BufferStorageFlags::Dynamic_Storage;
	m_transformBlock.create(desc);


	vx::gl::BufferDescription materialDesc;
	materialDesc.bufferType = vx::gl::BufferType::Shader_Storage_Buffer;
	materialDesc.size = sizeof(MaterialGPU) * s_maxMaterials;
	materialDesc.immutable = 1;
	materialDesc.flags = vx::gl::BufferStorageFlags::Write;
	m_materialBlock.create(materialDesc);

	{
		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Pixel_Pack_Buffer;
		desc.size = m_pColdData->m_windowResolution.x * m_pColdData->m_windowResolution.y * sizeof(F32) * 4;
		desc.immutable = 1;
		desc.flags = vx::gl::BufferStorageFlags::Read;
		m_pColdData->m_screenshotBuffer.create(desc);
	}

	createVoxelBuffer();

	{
		const auto maxRaySizeBytes = sizeof(CompressedRay) * 960 * 540;

		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Shader_Storage_Buffer;
		desc.size = maxRaySizeBytes;
		desc.immutable = 1;
		desc.flags = vx::gl::BufferStorageFlags::Read | vx::gl::BufferStorageFlags::Write;
		m_rayList.create(desc);
	}

	{
		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Shader_Storage_Buffer;
		desc.size = sizeof(RayLink) * 960 * 540 * 4;
		desc.immutable = 1;
		desc.flags = vx::gl::BufferStorageFlags::Read | vx::gl::BufferStorageFlags::Write;

		m_rayLinks.create(desc);
	}

	{
		vx::gl::BufferDescription vboDesc;
		vboDesc.bufferType = vx::gl::BufferType::Array_Buffer;
		vboDesc.size = sizeof(VoxelTrianglePair) * 50000;
		vboDesc.flags = vx::gl::BufferStorageFlags::Write;
		vboDesc.immutable = 1;
		m_voxelTrianglePairVbo.create(vboDesc);

		m_voxelTrianglePairVao.create();

		m_voxelTrianglePairVao.enableArrayAttrib(0);
		m_voxelTrianglePairVao.arrayAttribBinding(0, 0);
		m_voxelTrianglePairVao.arrayAttribFormatF(0, 3, 0, 0);

		m_voxelTrianglePairVao.enableArrayAttrib(1);
		m_voxelTrianglePairVao.arrayAttribBinding(1, 0);
		m_voxelTrianglePairVao.arrayAttribFormatF(1, 3, 0, sizeof(vx::float3));

		m_voxelTrianglePairVao.enableArrayAttrib(2);
		m_voxelTrianglePairVao.arrayAttribBinding(2, 0);
		m_voxelTrianglePairVao.arrayAttribFormatF(2, 3, 0, sizeof(vx::float3) * 2);

		m_voxelTrianglePairVao.enableArrayAttrib(3);
		m_voxelTrianglePairVao.arrayAttribBinding(3, 0);
		m_voxelTrianglePairVao.arrayAttribFormatF(3, 3, 0, sizeof(vx::float3) * 3);

		m_voxelTrianglePairVao.bindVertexBuffer(m_voxelTrianglePairVbo, 0, 0, sizeof(VoxelTrianglePair));
	}

	return true;
}

void RenderAspect::createVoxelBuffer()
{
	const F32 gridSize = 50.0f;
	const F32 gridHalfSize = (gridSize * ((float(s_voxelDimension) + 2.0f) / float(s_voxelDimension))) / 2.0f;

	const __m128 axisX = { 1, 0, 0, 0 };
	const __m128 axisY = { 0, 1, 0, 0 };

	auto voxelProjMatrix = vx::MatrixOrthographicOffCenterRH(-gridHalfSize, (float)gridHalfSize, -gridHalfSize, (float)gridHalfSize, 0.1f, (float)gridSize);

	VoxelBlock block;

	block.projectionMatrices[0] = voxelProjMatrix * vx::MatrixRotationAxis(axisY, vx::degToRad(90.0f)) * vx::MatrixTranslation(gridHalfSize, 0, 0);
	block.projectionMatrices[1] = voxelProjMatrix * vx::MatrixRotationAxis(axisX, vx::degToRad(90.0f)) * vx::MatrixTranslation(0, -gridHalfSize, 0);
	block.projectionMatrices[2] = voxelProjMatrix * vx::MatrixTranslation(0, 0, -gridHalfSize);

	block.dim = s_voxelDimension;
	block.halfDim = s_voxelDimension / 2;
	block.gridCellSize = gridHalfSize / block.halfDim;
	block.invGridCellSize = 1.0f / block.gridCellSize;

	vx::gl::BufferDescription desc;
	desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
	desc.size = sizeof(VoxelBlock);
	desc.immutable = 1;
	desc.pData = &block;
	m_voxelBlock.create(desc);
}

void RenderAspect::getShadowTransform(const Light &light, vx::mat4 *projMatrix, vx::mat4 *pvMatrices)
{
	__m128 p = vx::loadFloat(&light.m_position);
	*projMatrix = vx::MatrixPerspectiveFovRH(vx::degToRad(90.0), 1.0f, 1.0f, light.m_falloff);
	auto lightTranslationMatrix = vx::MatrixTranslation(-light.m_position.x, -light.m_position.y, -light.m_position.z);

	vx::mat4 viewMatrices[6];
	// X+
	vx::float4 up = { 0, -1, 0, 0 };
	vx::float4 dir = { 1, 0, 0, 0 };
	viewMatrices[0] = vx::MatrixLookToRH(p, vx::loadFloat(&dir), vx::loadFloat(&up));
	// X-
	up = { 0, -1, 0, 0 };
	dir = { -1, 0, 0, 0 };
	viewMatrices[1] = vx::MatrixLookToRH(p, vx::loadFloat(&dir), vx::loadFloat(&up));
	// Y+
	up = { 0, 0, 1, 0 };
	dir = vx::float4(0, 1, 0, 0);
	viewMatrices[2] = vx::MatrixLookToRH(p, vx::loadFloat(&dir), vx::loadFloat(&up));
	// Y-
	up = { 0, 0, -1, 0 };
	dir = vx::float4(0, -1, 0, 0);
	viewMatrices[3] = vx::MatrixLookToRH(p, vx::loadFloat(&dir), vx::loadFloat(&up));
	// Z+
	up = { 0, -1, 0, 0 };
	dir = vx::float4(0, 0, 1, 0);
	viewMatrices[4] = vx::MatrixLookToRH(p, vx::loadFloat(&dir), vx::loadFloat(&up));
	// Z-
	up = { 0, -1, 0, 0 };
	dir = vx::float4(0, 0, -1, 0);
	viewMatrices[5] = vx::MatrixLookToRH(p, vx::loadFloat(&dir), vx::loadFloat(&up));

	for (U32 i = 0; i < 6; ++i)
	{
		pvMatrices[i] = *projMatrix * viewMatrices[i];
	}
}

void RenderAspect::updateLightBuffer(const Light *pLights, U32 numLights)
{
	assert(numLights <= 10);

	LightDataBlock lightDataBlock;

	ShadowTransformBlock shadowTransforms;
	for (auto i = 0u; i < numLights; ++i)
	{
		/*vx::mat4 projMatrix;
		getShadowTransform(pLights[i], &projMatrix, shadowTransforms.pvMatrix + 6 * i);

		for (U32 j = 0; j < 6; ++j)
		{
		shadowTransforms.position[i * 6 + j] = vx::float4(pLights[i].m_position, 1.0f);
		}*/

		//lightDataBlock.u_lightData[i].projectionMatrix = projMatrix;
		lightDataBlock.u_lightData[i].position = vx::float4(pLights[i].m_position, 1.0f);
		lightDataBlock.u_lightData[i].falloff = pLights[i].m_falloff;
		lightDataBlock.u_lightData[i].surfaceRadius = pLights[i].m_surfaceRadius;
		lightDataBlock.u_lightData[i].lumen = pLights[i].m_lumen;

	}
	lightDataBlock.size = numLights;

	m_numPointLights = numLights;

	auto pLightData = m_lightDataBlock.map<LightDataBlock>(vx::gl::Map::Write_Only);
	*pLightData = lightDataBlock;

	//cudaUpdateLights((const cu::SphereLightData*)lightDataBlock.u_lightData, numLights);
}

void RenderAspect::initializeUniformBuffers()
{
	vx::gl::BufferDescription cameraDesc;
	cameraDesc.bufferType = vx::gl::BufferType::Uniform_Buffer;
	cameraDesc.size = sizeof(Camerablock);
#ifdef _VX_GL_45
	cameraDesc.immutable = 1;
	cameraDesc.flags = vx::gl::BufferStorageFlags::Write | vx::gl::BufferStorageFlags::Dynamic_Storage;
#else
	cameraDesc.usage = vx::gl::BufferDataUsage::Dynamic_Draw;
#endif
	m_cameraBuffer.create(cameraDesc);

	CamerablockStatic cameraBlockStatic;
	cameraBlockStatic.invProjectionMatrix = vx::MatrixInverse(m_renderContext.getProjectionMatrix());
	cameraBlockStatic.projectionMatrix = m_renderContext.getProjectionMatrix();
	cameraBlockStatic.orthoMatrix = m_renderContext.getOrthoMatrix();

	cameraDesc.size = sizeof(CamerablockStatic);
#ifdef _VX_GL_45
	cameraDesc.flags = vx::gl::BufferStorageFlags::None;
#else
	cameraDesc.usage = vx::gl::BufferDataUsage::Static_Read;
#endif
	cameraDesc.pData = &cameraBlockStatic;
	m_cameraBufferStatic.create(cameraDesc);

	LightDataBlock lightdata;
	lightdata.size = 0;
	vx::gl::BufferDescription lightDataDesc;
	lightDataDesc.bufferType = vx::gl::BufferType::Uniform_Buffer;
	lightDataDesc.size = sizeof(LightDataBlock);
#ifdef _VX_GL_45
	lightDataDesc.immutable = 1;
	lightDataDesc.flags = vx::gl::BufferStorageFlags::Write;
#else
	lightDataDesc.usage = vx::gl::BufferDataUsage::Dynamic_Draw;
#endif
	lightDataDesc.pData = &lightdata;
	m_lightDataBlock.create(lightDataDesc);

	{
		U64 handles[4];
		handles[0] = m_pColdData->m_gbufferAlbedoSlice.getTextureHandle();
		handles[1] = m_pColdData->m_gbufferNormalSlice.getTextureHandle();
		handles[2] = m_pColdData->m_gbufferSurfaceSlice.getTextureHandle();
		handles[3] = m_pColdData->m_gbufferNormalSliceSmall.getTextureHandle();

		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
		desc.size = sizeof(U64) * 4;
		desc.immutable = 1;
		desc.pData = handles;
		m_gbufferBlock.create(desc);
	}
}

void RenderAspect::createTextures()
{
	m_pColdData->m_textureManager.createBucket(1, vx::ushort3(1024, 1024, 10), 1, vx::gl::Texture_2D_Array, vx::gl::SRGBA8);
	m_pColdData->m_textureManager.createBucket(1, vx::ushort3(1024, 1024, 10), 1, vx::gl::Texture_2D_Array, vx::gl::SRGB8);
	m_pColdData->m_textureManager.createBucket(1, vx::ushort3(1024, 1024, 10), 1, vx::gl::Texture_2D_Array, vx::gl::RGB8);

	const auto numHandles = 3u;
	U64 handles[numHandles];
	handles[0] = m_pColdData->m_textureManager.createTexture(vx::ushort3(1024, 1024, 10), 1, vx::gl::Texture_2D_Array, vx::gl::SRGBA8);
	handles[1] = m_pColdData->m_textureManager.createTexture(vx::ushort3(1024, 1024, 10), 1, vx::gl::Texture_2D_Array, vx::gl::RGB8);
	handles[2] = m_pColdData->m_textureManager.createTexture(vx::ushort3(1024, 1024, 10), 1, vx::gl::Texture_2D_Array, vx::gl::SRGB8);

	//glTextureStorage2D(cmpTex, 1, GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC, 64, 64);

	//auto pHandles = std::make_unique<U64[]>(numHandles);
	for (U32 i = 0; i < numHandles; ++i)
	{
		m_pColdData->m_texturesGPU.insert(handles[i], i);
	}

	vx::gl::BufferDescription desc;
	desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
	desc.size = sizeof(U64) * numHandles;
#ifdef _VX_GL_45
	desc.immutable = 1;
#else
	desc.usage = vx::gl::BufferDataUsage::Static_Read;
#endif
	desc.pData = handles;
	m_textureBlock.create(desc);

	{
		vx::gl::TextureDescription desc;
		desc.type = vx::gl::Texture_2D;
		desc.size = vx::ushort3(m_pColdData->m_windowResolution.x, m_pColdData->m_windowResolution.y, 1);
		desc.miplevels = 1;
		desc.sparse = 0;

		desc.format = vx::gl::TextureFormat::RGB8;
		m_pColdData->m_gbufferAlbedoSlice.create(desc);
		m_pColdData->m_gbufferAlbedoSlice.makeTextureResident();

		desc.format = vx::gl::TextureFormat::RGB16F;
		m_pColdData->m_gbufferNormalSlice.create(desc);
		m_pColdData->m_gbufferNormalSlice.makeTextureResident();

		desc.format = vx::gl::TextureFormat::RGBA8;
		m_pColdData->m_gbufferSurfaceSlice.create(desc);
		m_pColdData->m_gbufferSurfaceSlice.makeTextureResident();

		desc.format = vx::gl::TextureFormat::DEPTH32;
		desc.type = vx::gl::Texture_2D;
		desc.size = vx::ushort3(m_pColdData->m_windowResolution.x, m_pColdData->m_windowResolution.y, 1);
		m_pColdData->m_gbufferDepthTexture.create(desc);
	}

	{
		vx::gl::TextureDescription desc;
		desc.type = vx::gl::Texture_3D;
		desc.size = vx::ushort3(s_voxelDimension, s_voxelDimension, s_voxelDimension);
		desc.miplevels = 1;
		desc.sparse = 0;
		desc.format = vx::gl::TextureFormat::R8;
		m_voxelTexture.create(desc);

		desc.format = vx::gl::TextureFormat::R32UI;
		m_voxelRayLinkOffsetTexture.create(desc);
		m_voxelRayLinkCountTexture.create(desc);
	}

	{
		vx::gl::TextureDescription desc;
		desc.type = vx::gl::Texture_2D;
		desc.size = vx::ushort3(960, 540, 1);
		desc.miplevels = 1;
		desc.sparse = 0;
		desc.format = vx::gl::TextureFormat::RGB16F;
		m_pColdData->m_gbufferNormalSliceSmall.create(desc);
		m_pColdData->m_gbufferNormalSliceSmall.makeTextureResident();

		desc.format = vx::gl::TextureFormat::DEPTH32;
		m_pColdData->m_gbufferDepthTextureSmall.create(desc);

	}

	{
		vx::gl::TextureDescription desc;
		desc.format = vx::gl::TextureFormat::R8;
		desc.type = vx::gl::Texture_2D;
		desc.size = vx::ushort3(1920, 1080, 1);
		desc.miplevels = 1;
		desc.sparse = 0;

		m_rayTraceShadowTexture.create(desc);
	}

	{
		vx::gl::TextureDescription desc;
		desc.format = vx::gl::TextureFormat::R8;
		desc.type = vx::gl::Texture_2D;
		desc.size = vx::ushort3(960, 540, 1);
		desc.miplevels = 1;
		desc.sparse = 0;
		m_rayTraceShadowTextureSmall.create(desc);

		m_rayTraceShadowTextureSmall.setWrapMode2D(vx::gl::TextureWrapMode::CLAMP_TO_EDGE, vx::gl::TextureWrapMode::CLAMP_TO_EDGE);
	}
}

void RenderAspect::createFrameBuffers()
{
	m_gbufferFB.create();
	m_gbufferFB.attachTexture(vx::gl::Attachment::Color0, m_pColdData->m_gbufferAlbedoSlice, 0);
	m_gbufferFB.attachTexture(vx::gl::Attachment::Color1, m_pColdData->m_gbufferNormalSlice, 0);
	m_gbufferFB.attachTexture(vx::gl::Attachment::Color2, m_pColdData->m_gbufferSurfaceSlice, 0);
	m_gbufferFB.attachTexture(vx::gl::Attachment::Depth, m_pColdData->m_gbufferDepthTexture, 0);

	GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glNamedFramebufferDrawBuffers(m_gbufferFB.getId(), 3, buffers);

	m_gbufferFBSmall.create();
	m_gbufferFBSmall.attachTexture(vx::gl::Attachment::Color0, m_pColdData->m_gbufferNormalSliceSmall, 0);
	m_gbufferFBSmall.attachTexture(vx::gl::Attachment::Depth, m_pColdData->m_gbufferDepthTextureSmall, 0);
	glNamedFramebufferDrawBuffers(m_gbufferFBSmall.getId(), 1, buffers);
}

bool RenderAspect::initialize(const std::string &dataDir, const RenderAspectDesc &desc)
{
	vx::gl::ContextDescription contextDesc = vx::gl::ContextDescription::create(*desc.window, desc.resolution, desc.fovRad, desc.z_near, desc.z_far, 4, 5, desc.vsync, desc.debug);
	if (!m_renderContext.initialize(contextDesc))
		return false;

	return initializeImpl(dataDir, desc.resolution, desc.debug, desc.targetMs, desc.pAllocator, desc.pProfiler, desc.pGraph);
}

bool RenderAspect::initializeImpl(const std::string &dataDir, const vx::uint2 &windowResolution, bool debug,
	F32 targetMs, vx::StackAllocator *pAllocator, Profiler2 *pProfiler, ProfilerGraph* pGraph)
{
	m_pColdData = std::make_unique<ColdData>();
	m_pColdData->m_windowResolution = windowResolution;

	m_allocator = vx::StackAllocator(pAllocator->allocate(5 MBYTE, 64), 5 MBYTE);
	m_scratchAllocator = vx::StackAllocator(pAllocator->allocate(1 MBYTE, 64), 1 MBYTE);

	if (debug)
	{
		vx::gl::Debug::initialize();
		vx::gl::Debug::setHighSeverityCallback(::debugCallback);
		vx::gl::Debug::enableCallback(true);
	}

	vx::gl::StateManager::enable(vx::gl::Capabilities::Framebuffer_sRGB);
	vx::gl::StateManager::enable(vx::gl::Capabilities::Texture_Cube_Map_Seamless);
	vx::gl::StateManager::setClearColor(0, 0, 0, 1);
	//m_stateManager.setViewport(0, 0, windowResolution.x, windowResolution.y);


	m_camera.setPosition(0, 2.5f, 15);

	if (!m_shaderManager.initialize(dataDir))
	{
		puts("Error initializing Shadermanager");
		return false;
	}

	createTextures();
	createFrameBuffers();

	initializeUniformBuffers();
	initializeBuffers();

	{
		TextureFile fontTexture;
		if (!fontTexture.loadFromFile((dataDir + "textures/verdana.png").c_str()))
			return false;

		auto ref = m_pColdData->m_textureManager.load(fontTexture, 1, 1);

		FontAtlas fontAtlas;
		if (!fontAtlas.loadFromFile((dataDir + "fonts/meta/VerdanaRegular.sdff").c_str()))
			return false;

		m_pColdData->m_font = Font(std::move(ref), std::move(fontAtlas));
	}

#if _VX_PROFILER
	auto textureIndex = *m_pColdData->m_texturesGPU.find(glGetTextureHandleARB(m_pColdData->m_font.getTextureEntry().getTextureId()));
	if (!pProfiler->initialize(&m_pColdData->m_font, m_shaderManager.getPipeline("text.pipe"), textureIndex, m_pColdData->m_windowResolution, pAllocator))
		return false;

	pGraph->initialize(m_shaderManager, targetMs);
#else
	VX_UNREFERENCED_PARAMETER(pProfiler);
	VX_UNREFERENCED_PARAMETER(pGraph);
	VX_UNREFERENCED_PARAMETER(targetMs);
	VX_UNREFERENCED_PARAMETER(pAllocator);
#endif

	return true;
}

void RenderAspect::writeTransform(const vx::Transform &transform, U32 elementId)
{
	//transform.m_rotation = vx::degToRad(transform.m_rotation);

	auto qRotation = vx::loadFloat(&transform.m_rotation);
	qRotation = vx::QuaternionRotationRollPitchYawFromVector(qRotation);
	auto packedRotation = packQRotation(qRotation);

	vx::TransformGpu t;
	t.translation = transform.m_translation;
	t.scaling = transform.m_scaling;
	t.packedQRotation = packedRotation;

	//glNamedBufferSubData(m_transformBlock.getId(), sizeof(vx::TransformGpu) * elementId, sizeof(vx::TransformGpu), &t);

	auto pTransforms = m_transformBlock.map<vx::TransformGpu>(vx::gl::Map::Write_Only);
	vx::memcpy(pTransforms.get() + elementId, t);
	pTransforms.unmap();
}

void RenderAspect::writeMeshInstanceIdBuffer(U32 elementId, U32 materialIndex)
{
	auto drawId = elementId | (materialIndex << 16);
	auto pMeshId = m_pColdData->m_meshIdVbo.map<U32>(vx::gl::Map::Write_Only);
	pMeshId[elementId] = drawId;
}

void RenderAspect::writeMeshInstanceToCommandBuffer(MeshEntry meshEntry, U32 index, U32 elementId)
{
	vx::gl::DrawElementsIndirectCommand cmd;
	cmd.count = meshEntry.indexCount;
	cmd.instanceCount = 1;
	cmd.firstIndex = meshEntry.firstIndex;
	cmd.baseVertex = 0;
	cmd.baseInstance = elementId;

	auto pCmd = (vx::gl::DrawElementsIndirectCommand*)m_commandBlock.map(vx::gl::Map::Write_Only);
	pCmd[index] = cmd;
	m_commandBlock.unmap();
}

void RenderAspect::updateBuffers(const MeshInstance *pInstances, U32 instanceCount, const vx::sorted_vector<const Material*, U32> &materialIndices, const vx::sorted_vector<vx::StringID64, MeshEntry> &meshEntries)
{
	if (instanceCount == 0)
	{
		return;
	}

	//m_bvh.create(pInstances, instanceCount, m_fileAspect, &m_meshVertexBlock, &m_bvhBlock);

	U32 totalInstanceCount = 0;
	U32 drawCount = 0;

	U32 batchIndexCount = 0;
	U32 batchIndexStart = 0;
	U32 batchInstanceCount = 0;
	U32 batchInstanceStart = 0;

	auto batchMeshSid = pInstances[0].getMeshSid();
	//auto pBatchMaterial = pInstances[0].getMaterialSid();

	//U32 firstIndex = 0u;
	batchIndexCount = meshEntries.find(batchMeshSid)->indexCount;

	for (auto i = 0u; i < instanceCount; ++i)
	{
		auto currentMeshSid = pInstances[i].getMeshSid();
		auto meshEntry = meshEntries.find(currentMeshSid);
		auto pCurrentMaterial = m_fileAspect.getMaterial(pInstances[i].getMaterialSid());
		auto materialIndex = *materialIndices.find(pCurrentMaterial);

		auto pMesh = m_fileAspect.getMesh(currentMeshSid);
		U32 vertexCount = pMesh->getVertexCount();
		auto pVertices = pMesh->getVertices();
		vx::float4 max(-FLT_MAX, -FLT_MAX, -FLT_MAX, 0), min(FLT_MAX, FLT_MAX, FLT_MAX, 0);
		for (U32 j = 0; j < vertexCount; ++j)
		{
			max.x = fmaxf(pVertices[j].position.x, max.x);
			max.y = fmaxf(pVertices[j].position.y, max.y);
			max.z = fmaxf(pVertices[j].position.z, max.z);

			min.x = fminf(pVertices[j].position.x, min.x);
			min.y = fminf(pVertices[j].position.y, min.y);
			min.z = fminf(pVertices[j].position.z, min.z);
		}

		U16 elementId = i;

		U32 index = elementId * 2;
		//pAABB[index] = min;
		//pAABB[index + 1] = max;

		auto transform = pInstances[i].getTransform();
		transform.m_rotation = vx::degToRad(transform.m_rotation);

		writeTransform(transform, elementId);
		writeMeshInstanceIdBuffer(elementId, materialIndex);
		writeMeshInstanceToCommandBuffer(*meshEntry, elementId, elementId);

		++batchInstanceCount;
		++totalInstanceCount;
	}
}

void RenderAspect::writeMaterialToBuffer(const Material *pMaterial, U32 offset)
{
	auto getTextureGpuEntry = [&](const TextureRef &ref)
	{
		auto handle = m_pColdData->m_textureManager.getTextureHandle(ref);
		auto it = m_pColdData->m_texturesGPU.find(handle);
		//assert(it != m_texturesGPU.end());

		auto slice = ref.getSlice();
		auto index = *it;

		U32 entry = index | (slice << 16);

		return entry;
	};

	auto &albedoRef = pMaterial->getAlbedoRef();
	auto &normalRef = pMaterial->getNormalRef();
	auto &surfaceRef = pMaterial->getSurfaceRef();

	U32 hasNormalmap = (normalRef.isValid());

	auto pMaterialGPU = m_materialBlock.mapRange<MaterialGPU>(sizeof(MaterialGPU) * offset, sizeof(MaterialGPU), vx::gl::MapRange::Write);
	//auto pMaterialGPU = (MaterialGPU*)glMapNamedBufferRange(m_materialBlock.getId(), sizeof(MaterialGPU) * offset, sizeof(MaterialGPU), GL_MAP_WRITE_BIT);
	pMaterialGPU->indexAlbedo = getTextureGpuEntry(albedoRef);
	//printf("normals\n");
	if (hasNormalmap != 0)
		pMaterialGPU->indexNormal = getTextureGpuEntry(normalRef);
	//printf("surface\n");
	pMaterialGPU->indexSurface = getTextureGpuEntry(surfaceRef);
	pMaterialGPU->hasNormalMap = hasNormalmap;
	pMaterialGPU.unmap();
}

void RenderAspect::createMaterial(Material* pMaterial)
{
	assert(pMaterial);

	auto pAlbedoTex = m_fileAspect.getTextureFile(pMaterial->m_textureSid[0]);
	assert(pAlbedoTex);
	auto albedoRef = m_pColdData->m_textureManager.load(*pAlbedoTex, 1, 1);
	VX_ASSERT(albedoRef.isValid());

	auto pNormalTex = m_fileAspect.getTextureFile(pMaterial->m_textureSid[1]);
	assert(pNormalTex);
	auto normalRef = m_pColdData->m_textureManager.load(*pNormalTex, 1, 0);
	VX_ASSERT(normalRef.isValid());

	auto pSurfaceTex = m_fileAspect.getTextureFile(pMaterial->m_textureSid[2]);
	assert(pSurfaceTex);
	auto surfaceRef = m_pColdData->m_textureManager.load(*pSurfaceTex, 1, 1);
	VX_ASSERT(surfaceRef.isValid());

	pMaterial->setTextures(std::move(albedoRef), std::move(normalRef), std::move(surfaceRef));
}

void RenderAspect::loadCurrentScene(const Scene* pScene)
{
	auto &sceneMaterial = pScene->getMaterials();
	auto numMaterials = pScene->getMaterialCount();

	assert(m_materialCount + numMaterials <= s_maxMaterials);

	auto materialOffset = 0u;
	m_materialIndices.reserve(numMaterials);
	for (auto i = 0u; i < numMaterials; ++i)
	{
		auto pMaterial = sceneMaterial[i];
		createMaterial(pMaterial);

		writeMaterialToBuffer(pMaterial, m_materialCount);

		m_materialIndices.insert(pMaterial, m_materialCount);
		++m_materialCount;
	}

	m_meshEntries.reserve(pScene->getMeshes().size());

	updateLightBuffer(pScene->getLights(), pScene->getLightCount());
	updateMeshBuffer(pScene->getMeshes());
	updateBuffers(pScene->getMeshInstances(), pScene->getMeshInstanceCount(), m_materialIndices, m_meshEntries);

	updateNavMeshBuffer(pScene->getNavMesh());

	m_meshInstanceCountStatic = pScene->getMeshInstanceCount();

	m_meshInstancesCountTotal = m_meshInstanceCountStatic + m_meshInstanceCountDynamic;

	//	auto pInstances = pScene->getMeshInstances();
	//auto instanceCount = pScene->getMeshInstanceCount();

	//m_bvhDynamic.create(pInstances, instanceCount, m_fileAspect, &m_meshVertexBlock, &m_bvhBlock, &m_scratchAllocator);

	//printf("%u\n", m_pCurrentScene->getVertexCount());

	//auto pPlayerSpawn = m_pCurrentScene->getPlayerSpawn();
	//m_camera.setPosition(pPlayerSpawn.x, pPlayerSpawn.y, pPlayerSpawn.z);
}

void RenderAspect::writeMeshToVertexBuffer(const vx::StringID64 &meshSid, const vx::Mesh* pMesh, U32 *vertexOffsetGpu, U32 *indexOffsetGpu)
{
	/*auto pPosition = pMesh->getVertices();
	auto pNormals = pMesh->getNormals();
	auto pTangents = pMesh->getTangents();
	auto pBitangents = pMesh->getBitangents();
	auto pTexCoords = pMesh->getTexCoords();
	auto vertexCount = pMesh->getVertexCount();

	auto vertexSizeBytes = sizeof(VertexPNTUV) * vertexCount;
	VertexPNTUV* pVertices = (VertexPNTUV*)m_pColdData->m_allocator.allocate(vertexSizeBytes);
	VX_ASSERT(pVertices, "out of memory");

	//////////////////////////////
	for (auto j = 0u; j < vertexCount; ++j)
	{
	VertexPNTUV vertex;
	vertex.position = vx::float4(pPosition[j], 1.0f);
	vertex.normal = pNormals[j];
	vertex.tangent = pTangents[j];
	vertex.uv = pTexCoords[j];

	F32 w = vx::dot(vx::cross(vertex.normal, vertex.tangent), pBitangents[j]);
	if (w < 0.0f)
	{
	vertex.position.w = -1.0f;
	}

	pVertices[j] = vertex;
	}

	U32 offsetBytes = *vertexOffset * sizeof(VertexPNTUV);
	auto pGpuVertices = m_pColdData->m_meshVbo.mapRange<VertexPNTUV>(offsetBytes, vertexSizeBytes, vx::gl::MapRange::Write);
	VX_ASSERT(pGpuVertices, "Could not access vertex buffer !");
	//pGpuVertices[*vertexOffset + j] = vertex;
	memcpy(pGpuVertices, pVertices, vertexSizeBytes);
	m_pColdData->m_meshVbo.unmap();

	//////////////////////////////

	m_pColdData->m_allocator.clear();

	auto indexCount = pMesh->getIndexCount();
	U32 indexSizeBytes = sizeof(U32) * indexCount;
	U32* pIndices = (U32*)m_pColdData->m_allocator.allocate(indexSizeBytes);

	//////////////////////////////

	auto meshIndices = pMesh->getIndices();

	for (auto j = 0u; j < indexCount; ++j)
	{
	pIndices[j] = meshIndices[j] + *vertexOffset;
	}

	U32 offsetIndicesBytes = sizeof(U32) * (*indexOffset);
	auto pGpuIndices = (U32*)m_pColdData->m_meshIbo.mapRange(offsetIndicesBytes, indexSizeBytes, vx::gl::MapRange::Write);
	memcpy(pGpuIndices, pIndices, indexSizeBytes);
	m_pColdData->m_meshIbo.unmap();

	//////////////////////////////

	m_pColdData->m_allocator.clear();

	MeshEntry entry;
	entry.firstIndex = *indexOffset;
	entry.indexCount = indexCount;
	meshEntries->insert(meshSid, entry);

	*vertexOffset += vertexCount;
	*indexOffset += indexCount;*/

	auto marker = m_scratchAllocator.getMarker();
	SCOPE_EXIT
	{
		m_scratchAllocator.clear(marker);
	};

	auto vertexCount = pMesh->getVertexCount();
	auto vertexSizeBytes = sizeof(VertexPNTUV) * vertexCount;
	VertexPNTUV* pVertices = (VertexPNTUV*)m_scratchAllocator.allocate(vertexSizeBytes);

	auto indexCount = pMesh->getIndexCount();
	U32 indexSizeBytes = sizeof(U32) * indexCount;
	U32* pIndices = (U32*)m_scratchAllocator.allocate(indexSizeBytes);

	U32 offsetBytes = *vertexOffsetGpu * sizeof(VertexPNTUV);
	U32 offsetIndicesBytes = sizeof(U32) * (*indexOffsetGpu);

	U32 tmpOffset = 0;
	writeMeshToBuffer(meshSid, pMesh, pVertices, pIndices, &tmpOffset, &tmpOffset, vertexOffsetGpu, indexOffsetGpu);

	auto pGpuVertices = m_pColdData->m_meshVbo.mapRange<VertexPNTUV>(offsetBytes, vertexSizeBytes, vx::gl::MapRange::Write);
	memcpy(pGpuVertices.get(), pVertices, vertexSizeBytes);

	auto pGpuIndices = m_pColdData->m_meshIbo.mapRange<U32>(offsetIndicesBytes, indexSizeBytes, vx::gl::MapRange::Write);
	memcpy(pGpuIndices.get(), pIndices, indexSizeBytes);

	//*vertexOffset += vertexCount;
	//*indexOffset += indexCount;
}

void RenderAspect::writeMeshToBuffer(const vx::StringID64 &meshSid, const vx::Mesh* pMesh, VertexPNTUV* pVertices, U32* pIndices, U32* vertexOffset, U32* indexOffset, U32 *vertexOffsetGpu, U32 *indexOffsetGpu)
{
	auto pMeshVertices = pMesh->getVertices();
	auto vertexCount = pMesh->getVertexCount();

	//////////////////////////////

	for (auto j = 0u; j < vertexCount; ++j)
	{
		VertexPNTUV vertex;
		vertex.position = vx::float4(pMeshVertices[j].position, 1.0f);
		vertex.normal = pMeshVertices[j].normal;
		vertex.tangent = pMeshVertices[j].tangent;
		vertex.uv = pMeshVertices[j].texCoords;

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
	m_meshEntries.insert(meshSid, entry);

	//////////////////////////////
	*vertexOffsetGpu += vertexCount;
	*vertexOffset += vertexCount;
	*indexOffsetGpu += indexCount;
	*indexOffset += indexCount;
}

void RenderAspect::writeMeshesToVertexBuffer(const vx::StringID64* meshSid, const vx::Mesh** pMesh, U32 count, U32 *vertexOffsetGpu, U32 *indexOffsetGpu)
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

	auto marker = m_scratchAllocator.getMarker();
	SCOPE_EXIT
	{
		m_scratchAllocator.clear(marker);
	};

	auto vertexSizeBytes = sizeof(VertexPNTUV) * vertexCount;
	auto indexSizeBytes = sizeof(U32) * indexCount;
	VertexPNTUV* pVertices = (VertexPNTUV*)m_scratchAllocator.allocate(vertexSizeBytes);
	U32* pIndices = (U32*)m_scratchAllocator.allocate(indexSizeBytes);
	VX_ASSERT(pVertices && pIndices);

	U32 vertexOffset = 0, indexOffset = 0;
	for (U32 i = 0; i < count; ++i)
	{
		writeMeshToBuffer(meshSid[i], pMesh[i], pVertices, pIndices, &vertexOffset, &indexOffset, vertexOffsetGpu, indexOffsetGpu);
	}

	// upload to gpu

	auto pGpuVertices = m_pColdData->m_meshVbo.mapRange<VertexPNTUV>(offsetBytes, vertexSizeBytes, vx::gl::MapRange::Write);
	memcpy(pGpuVertices.get(), pVertices, vertexSizeBytes);

	auto pGpuIndices = m_pColdData->m_meshIbo.mapRange<U32>(offsetIndicesBytes, indexSizeBytes, vx::gl::MapRange::Write);
	memcpy(pGpuIndices.get(), pIndices, indexSizeBytes);
}

void RenderAspect::updateMeshBuffer(const vx::sorted_vector<vx::StringID64, const vx::Mesh*> &meshes)
{
	U32 totalVertexCount = 0;
	U32 totalIndexCount = 0;
	auto meshCount = meshes.size();
	writeMeshesToVertexBuffer(meshes.keys(), meshes.data(), meshCount, &totalVertexCount, &totalIndexCount);
}

void RenderAspect::shutdown(const HWND hwnd)
{
	m_pColdData.reset(nullptr);
	m_renderContext.shutdown(hwnd);
}

void RenderAspect::update()
{
	auto projectionMatrix = m_renderContext.getProjectionMatrix();

	Camerablock block;
	m_camera.getViewMatrix(block.viewMatrix);
	block.pvMatrix = projectionMatrix * block.viewMatrix;
	block.inversePVMatrix = vx::MatrixInverse(block.pvMatrix);
	block.cameraPosition = m_camera.getPosition();

	auto p = m_cameraBuffer.mapRange<Camerablock>(0, sizeof(Camerablock), vx::gl::MapRange::Write);
	vx::memcpy(p.get(), block);
}

void RenderAspect::updateTransform(U16 index, const vx::TransformGpu &transform)
{
	auto mappedTransformPtr = m_transformBlock.mapRange<vx::TransformGpu>(sizeof(vx::TransformGpu) * index, sizeof(vx::TransformGpu), vx::gl::MapRange::Write);
	*mappedTransformPtr = transform;
}

void RenderAspect::updateTransforms(const vx::TransformGpu* transforms, U32 offsetCount, U32 count)
{
	auto offsetInBytes = offsetCount * sizeof(vx::TransformGpu);
	auto sizeInBytes = count * sizeof(vx::TransformGpu);

	//glNamedBufferSubData(m_transformBlock.getId(), offsetInBytes, sizeInBytes, transforms);

	auto p = m_transformBlock.mapRange<vx::TransformGpu>(offsetInBytes, sizeInBytes, vx::gl::MapRange::Write);
	::memcpy(p.get(), transforms, sizeInBytes);
}

namespace
{
	/// Inverse of bitsep2.
	inline U32 bitcomp2(U32 x)
	{
		// http://fgiesen.wordpress.com/2009/12/13/decoding-morton-codes/
		x &= 0x09249249;                  // x = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
		x = (x ^ (x >> 2)) & 0x030c30c3; // x = ---- --98 ---- 76-- --54 ---- 32-- --10
		x = (x ^ (x >> 4)) & 0x0300f00f; // x = ---- --98 ---- ---- 7654 ---- ---- 3210
		x = (x ^ (x >> 8)) & 0xff0000ff; // x = ---- --98 ---- ---- ---- ---- 7654 3210
		x = (x ^ (x >> 16)) & 0x000003ff; // x = ---- ---- ---- ---- ---- --98 7654 3210
		return x;
	}

	/// 3 dimensions from morton code.
	inline vx::uint3 bitunzip3(U32 c)
	{
		return vx::uint3(bitcomp2(c), bitcomp2(c >> 1), bitcomp2(c >> 2));
	}

	vx::float3 decodeNormal(const vx::float2 &enc)
	{
		float scale = 1.7777f;
		vx::float3 nn = vx::float3(enc, 1.0) * vx::float3(2 * scale, 2 * scale, 0) + vx::float3(-scale, -scale, 1);

		float g = 2.0 / dot(nn, nn);

		vx::float3 n;

		n.x = g * nn.x;
		n.y = g * nn.y;
		n.z = g - 1;

		return n;
	}
}

void RenderAspect::render(Profiler2* pProfiler, ProfilerGraph* pGraph)
{
	pProfiler->pushGpuMarker("render()");
	pGraph->startGpu();

	clearTextures();

	{
		auto pCounter = m_atomicCounter.map<U32>(vx::gl::Map::Write_Only);
		auto ptr = pCounter.get();
		ptr[0] = 0; // u_rayCount
		ptr[1] = 0; // u_rayLinkCount
		ptr[2] = 0; // pair counter
		ptr[3] = 0;
		pCounter.unmap();
	}

	auto ptrRayOffsets = m_rayOffsetBlock.map<U32>(vx::gl::Map::Write_Only);
	*ptrRayOffsets = 0;
	ptrRayOffsets.unmap();

	auto ptrVoxelTrianglePair = m_voxelTrianglePairCmdBuffer.map<DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
	ptrVoxelTrianglePair->count = 0;
	ptrVoxelTrianglePair.unmap();

	BufferBindingManager::bindBaseUniform(0, m_cameraBuffer.getId());
	BufferBindingManager::bindBaseUniform(1, m_lightDataBlock.getId());
	BufferBindingManager::bindBaseUniform(2, m_cameraBufferStatic.getId());
	BufferBindingManager::bindBaseUniform(3, m_gbufferBlock.getId());
	BufferBindingManager::bindBaseUniform(4, m_voxelBlock.getId());

	BufferBindingManager::bindBaseShaderStorage(0, m_transformBlock.getId());
	BufferBindingManager::bindBaseShaderStorage(1, m_materialBlock.getId());
	BufferBindingManager::bindBaseShaderStorage(2, m_textureBlock.getId());
	BufferBindingManager::bindBaseShaderStorage(3, m_rayList.getId());
	BufferBindingManager::bindBaseShaderStorage(4, m_rayLinks.getId());
	BufferBindingManager::bindBaseShaderStorage(5, m_bvhBlock.getId());
	BufferBindingManager::bindBaseShaderStorage(6, m_meshVertexBlock.getId());
	BufferBindingManager::bindBaseShaderStorage(7, m_rayOffsetBlock.getId());
	BufferBindingManager::bindBaseShaderStorage(8, m_voxelTrianglePairCmdBuffer.getId());
	BufferBindingManager::bindBaseShaderStorage(9, m_voxelTrianglePairVbo.getId());

	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, m_atomicCounter.getId());

	vx::gl::StateManager::setClearColor(0, 0, 0, 0);

	pProfiler->pushGpuMarker("zero rays()");
	zeroRayBuffer();
	pProfiler->popGpuMarker();

	// voxelize
	pProfiler->pushGpuMarker("voxelize()");
	binaryVoxelize();
	pProfiler->popGpuMarker();

	if (dev::g_toggleRender == 0)
	{
		pProfiler->pushGpuMarker("create gbuffer()");
		createGBuffer();
		pProfiler->popGpuMarker();

		createGBufferSmall();

		vx::gl::StateManager::setClearColor(0.1f, 0.1f, 0.1f, 1);

		vx::gl::StateManager::bindFrameBuffer(0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// only do ray tracing if we actually have stuff
		if (m_meshInstancesCountTotal != 0)
		{
			vx::gl::StateManager::bindVertexArray(m_emptyVao.getId());

			pProfiler->pushGpuMarker("ray trace voxel()");
			createRays();
			pProfiler->popGpuMarker();

			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			// create offsets for raylinks
			pProfiler->pushGpuMarker("create link offsets");
			createRayLinkOffsets();
			pProfiler->popGpuMarker();

			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			pProfiler->pushGpuMarker("create ray links()");
			createRayLinks();
			pProfiler->popGpuMarker();

			pProfiler->pushGpuMarker("test voxel triangles()");
			testVoxelTriangles();
			pProfiler->popGpuMarker();

			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_COMMAND_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

			pProfiler->pushGpuMarker("test ray triangles()");
			testRayTriangles();
			pProfiler->popGpuMarker();

			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			renderFinalImage();

			/*{
				U32 pairCount = 0;
				auto pCounter = m_rayAtomicCounter.map< U32 >(vx::gl::Map::Read_Only);

				pairCount = pCounter[1];
				pCounter.unmap();

				U32 offsetCount = 0;
				auto offset = m_rayOffsetBlock.map<U32>(vx::gl::Map::Read_Only);
				offsetCount = *offset;
				offset.unmap();

				printf("%u %u\n", pairCount, offsetCount);
				}*/
		}
	}
	else
	{
		renderForward();
	}

	// nav nodes
	if (dev::g_showNavGraph != 0)
	{
		renderNavGraph();
	}

	pProfiler->popGpuMarker();
	pGraph->endGpu();

#if _VX_PROFILER
	renderProfiler(pProfiler, pGraph);
#endif

	m_renderContext.swapBuffers();
}

void RenderAspect::clearTextures()
{
	m_rayTraceShadowTexture.clearImage(0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
	m_rayTraceShadowTextureSmall.clearImage(0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

	m_voxelTexture.clearImage(0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
	m_voxelRayLinkCountTexture.clearImage(0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
	m_voxelRayLinkOffsetTexture.clearImage(0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
}

void RenderAspect::zeroRayBuffer()
{
	auto pPipeline = m_shaderManager.getPipeline("zero_rays.pipe");
	vx::gl::StateManager::bindPipeline(pPipeline->getId());
	vx::gl::StateManager::bindVertexArray(m_emptyVao.getId());

	glDrawArraysInstanced(GL_TRIANGLES, 0, 960, 540);
}

void RenderAspect::binaryVoxelize()
{
	vx::gl::StateManager::setViewport(0, 0, 128, 128);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);

	auto pPipeline = m_shaderManager.getPipeline("voxelize.pipe");

	vx::gl::StateManager::bindFrameBuffer(0);
	vx::gl::StateManager::bindPipeline(pPipeline->getId());
	vx::gl::StateManager::bindVertexArray(m_meshVao.getId());

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);

	ImageBindingManager::bind(0, m_voxelTexture.getId(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R8);

	m_commandBlock.bind();
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, m_meshInstancesCountTotal, sizeof(vx::gl::DrawElementsIndirectCommand));

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);
}

void RenderAspect::createGBuffer()
{
	vx::gl::StateManager::setClearColor(0, 0, 0, 0);
	vx::gl::StateManager::setViewport(0, 0, 1920, 1080);
	vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);

	vx::gl::StateManager::bindFrameBuffer(m_gbufferFB.getId());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto pPipeline = m_shaderManager.getPipeline("create_gbuffer.pipe");
	vx::gl::StateManager::bindPipeline(pPipeline->getId());

	vx::gl::StateManager::bindVertexArray(m_meshVao.getId());

	m_commandBlock.bind();
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, m_meshInstancesCountTotal, sizeof(vx::gl::DrawElementsIndirectCommand));

	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);
}

void RenderAspect::createGBufferSmall()
{
	vx::gl::StateManager::setClearColor(0, 0, 0, 0);
	vx::gl::StateManager::setViewport(0, 0, 1920 / 2, 1080 / 2);
	vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);

	vx::gl::StateManager::bindFrameBuffer(m_gbufferFBSmall.getId());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto pPipeline = m_shaderManager.getPipeline("create_gbuffer_small.pipe");
	vx::gl::StateManager::bindPipeline(pPipeline->getId());

	vx::gl::StateManager::bindVertexArray(m_meshVao.getId());

	m_commandBlock.bind();
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, m_meshInstancesCountTotal, sizeof(vx::gl::DrawElementsIndirectCommand));

	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);
}

void RenderAspect::createRays()
{
	auto pPipeline = m_shaderManager.getPipeline("ray_trace_voxel.pipe");
	vx::gl::StateManager::bindPipeline(pPipeline->getId());

	ImageBindingManager::bind(0, m_rayTraceShadowTextureSmall.getId(), 0, 0, 0, GL_WRITE_ONLY, GL_R8);
	ImageBindingManager::bind(1, m_voxelTexture.getId(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_R8);
	ImageBindingManager::bind(2, m_voxelRayLinkCountTexture.getId(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32UI);

	glDrawArraysInstanced(GL_TRIANGLES, 0, 960, 540);
}

void RenderAspect::createRayLinkOffsets()
{
	auto pPipeline = m_shaderManager.getPipeline("create_ray_link_offsets.pipe");
	vx::gl::StateManager::bindPipeline(pPipeline->getId());

	ImageBindingManager::bind(2, m_voxelRayLinkCountTexture.getId(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);
	ImageBindingManager::bind(3, m_voxelRayLinkOffsetTexture.getId(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32UI);

	glDispatchCompute(s_voxelDimension / 2, s_voxelDimension / 2, s_voxelDimension / 2);
}

void RenderAspect::createRayLinks()
{
	m_voxelRayLinkCountTexture.clearImage(0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);

	glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);

	auto pPipeline = m_shaderManager.getPipeline("create_ray_links.pipe");
	vx::gl::StateManager::bindPipeline(pPipeline->getId());

	ImageBindingManager::bind(0, m_rayTraceShadowTextureSmall.getId(), 0, 0, 0, GL_WRITE_ONLY, GL_R8);
	ImageBindingManager::bind(1, m_voxelTexture.getId(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_R8);
	ImageBindingManager::bind(2, m_voxelRayLinkCountTexture.getId(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32UI);
	ImageBindingManager::bind(3, m_voxelRayLinkOffsetTexture.getId(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);

	glDrawArraysInstanced(GL_TRIANGLES, 0, 960, 540);
}

void RenderAspect::testVoxelTriangles()
{
	vx::gl::StateManager::setViewport(0, 0, 128, 128);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);

	vx::gl::StateManager::bindFrameBuffer(0);
	vx::gl::StateManager::bindVertexArray(m_meshVao.getId());

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);

	auto pPipeline = m_shaderManager.getPipeline("voxel_test_triangles.pipe");
	vx::gl::StateManager::bindPipeline(pPipeline->getId());

	m_commandBlock.bind();
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, m_meshInstancesCountTotal, sizeof(vx::gl::DrawElementsIndirectCommand));

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
}

void RenderAspect::testRayTriangles()
{
	ImageBindingManager::bind(1, m_rayTraceShadowTextureSmall.getId(), 0, 0, 0, GL_WRITE_ONLY, GL_R8);
	ImageBindingManager::bind(2, m_voxelRayLinkCountTexture.getId(), 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32I);
	ImageBindingManager::bind(3, m_voxelRayLinkOffsetTexture.getId(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);

	vx::gl::StateManager::setViewport(0, 0, 960, 540);
	vx::gl::StateManager::enable(vx::gl::Capabilities::Blend);

	auto pPipeline = m_shaderManager.getPipeline("test_pair_rayLinks.pipe");
	vx::gl::StateManager::bindPipeline(pPipeline->getId());
	vx::gl::StateManager::bindVertexArray(m_voxelTrianglePairVao.getId());

	m_voxelTrianglePairCmdBuffer.bind();
	glDrawArraysIndirect(GL_POINTS, 0);

	vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);
}

void RenderAspect::renderFinalImage()
{
	vx::gl::StateManager::setViewport(0, 0, 1920, 1080);
	// draw final image
	ImageBindingManager::bind(0, m_rayTraceShadowTexture.getId(), 0, 0, 0, GL_READ_ONLY, GL_R8);
	ImageBindingManager::bind(1, m_rayTraceShadowTextureSmall.getId(), 0, 0, 0, GL_READ_ONLY, GL_R8);

	auto pPipeline = m_shaderManager.getPipeline("ray_trace_draw.pipe");
	vx::gl::StateManager::bindPipeline(pPipeline->getId());
	glDrawArrays(GL_POINTS, 0, 1);
}

void RenderAspect::renderForward()
{
	vx::gl::StateManager::bindFrameBuffer(0);

	vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto pPipeline = m_shaderManager.getPipeline("forward_render.pipe");
	vx::gl::StateManager::bindPipeline(pPipeline->getId());
	vx::gl::StateManager::bindVertexArray(m_meshVao.getId());

	m_commandBlock.bind();
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, m_meshInstancesCountTotal, sizeof(vx::gl::DrawElementsIndirectCommand));

	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);
}

void RenderAspect::renderProfiler(Profiler2* pProfiler, ProfilerGraph* pGraph)
{
	vx::gl::StateManager::enable(vx::gl::Capabilities::Blend);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	pProfiler->render();
	pGraph->render();

	vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);
	vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);
}

void RenderAspect::renderNavGraph()
{
	// nav mesh
	/*m_stateManager.enable(vx::gl::Capabilities::Blend);
	m_stateManager.disable(vx::gl::Capabilities::Depth_Test);

	auto pPipeline = m_shaderManager.getPipeline("navmesh.pipe");
	vx::float3 color(0, 0, 1);
	glProgramUniform3fv(pPipeline->getFragmentShader(), 0, 1, color);

	m_stateManager.bindPipeline(pPipeline->getId());
	m_stateManager.bindVertexArray(m_navmeshVao.getId());
	glDrawElements(GL_TRIANGLES, m_navmeshIndexCount, GL_UNSIGNED_SHORT, 0);

	m_stateManager.disable(vx::gl::Capabilities::Blend);
	m_stateManager.enable(vx::gl::Capabilities::Depth_Test);*/

	vx::gl::StateManager::enable(vx::gl::Capabilities::Blend);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);

	glPointSize(3.f);

	auto pPipeline = m_shaderManager.getPipeline("navmesh.pipe");

	vx::float3 color(1, 0, 0);
	glProgramUniform3fv(pPipeline->getFragmentShader(), 0, 1, color);

	vx::gl::StateManager::bindPipeline(pPipeline->getId());
	vx::gl::StateManager::bindVertexArray(m_navNodesVao.getId());
	glDrawElements(GL_POINTS, m_navNodesCount, GL_UNSIGNED_SHORT, 0);

	// connections
	vx::gl::StateManager::bindVertexArray(m_navConnectionsVao.getId());
	glDrawElements(GL_LINES, m_navConnectionCount, GL_UNSIGNED_SHORT, 0);

	vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);
	vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);
}

U16 RenderAspect::addActorToBuffer(const vx::Transform &transform, const vx::StringID64 &mesh, const vx::StringID64 &material, const Scene* pScene)
{
	auto itMesh = m_meshEntries.find(mesh);
	if (itMesh == m_meshEntries.end())
	{
		auto &sceneMeshes = pScene->getMeshes();
		auto itSceneMesh = sceneMeshes.find(mesh);

		U32 vertexOffset = m_meshVertexCountDynamic + s_maxVerticesStatic;
		U32 indexOffset = m_meshIndexCountDynamic + s_maxIndicesStatic;
		writeMeshToVertexBuffer(mesh, (*itSceneMesh), &vertexOffset, &indexOffset);

		m_meshVertexCountDynamic += (*itSceneMesh)->getVertexCount();
		m_meshIndexCountDynamic += (*itSceneMesh)->getIndexCount();

		itMesh = m_meshEntries.find(mesh);

		/*vx::float3 min(FLT_MAX), max(-FLT_MAX);
		auto pp = (*itSceneMesh)->getVertices();
		for (U32 i = 0; i < (*itSceneMesh)->getVertexCount(); ++i)
		{
		min = vx::min(min, pp[i]);
		max = vx::max(max, pp[i]);
		}

		F32 height = max.y - min.y;
		F32 d = max.z - min.z;*/
	}

	auto pMaterial = m_fileAspect.getMaterial(material);

	// add material
	auto itMaterial = m_materialIndices.find(pMaterial);
	if (itMaterial == m_materialIndices.end())
	{
		createMaterial(pMaterial);

		writeMaterialToBuffer(pMaterial, m_materialCount);

		itMaterial = m_materialIndices.insert(pMaterial, m_materialCount);

		++m_materialCount;
	}

	// create transform and draw command
	auto materialIndex = *itMaterial;

	U32 elementId = s_meshMaxInstancesStatic + m_meshInstanceCountDynamic;
	U16 index = m_meshInstancesCountTotal;

	writeTransform(transform, elementId);
	writeMeshInstanceIdBuffer(elementId, materialIndex);
	writeMeshInstanceToCommandBuffer(*itMesh, index, elementId);

	++m_meshInstanceCountDynamic;

	m_meshInstancesCountTotal = m_meshInstanceCountStatic + m_meshInstanceCountDynamic;

	return elementId;
}

U16 RenderAspect::getActorGpuIndex()
{
	U32 elementId = s_meshMaxInstancesStatic + m_meshInstanceCountDynamic;

	VX_ASSERT(U16(elementId) == elementId);

	++m_meshInstanceCountDynamic;

	return elementId;
}

void RenderAspect::keyPressed(U16 key)
{
	switch (key)
	{
	case VK_F10:
		takeScreenshot();
		break;
	}
}

void RenderAspect::keyReleased(U16 key)
{
	UNREFERENCED_PARAMETER(key);
}

// pData aligned to 16 bytes
void RenderAspect::writeScreenshot(const vx::uint2 &resolution, vx::float4 *pData)
{
	//std::unique_ptr<vx::float4[]> ptr(pData);

	auto size = resolution.x * resolution.y * 3;
	std::unique_ptr<U8[]> pngData = std::make_unique<U8[]>(size);

	const __m128 vToUINT8 = { 255.0f, 255.0f, 255.0f, 255.0f };
	//const __m128 invGamma = { 1.0f / 2.2f, 1.0f / 2.2f, 1.0f / 2.2f, 0.0f };

	for (U32 i = 0, j = 0; i < size; i += 3, ++j)
	{
		__m128 vTmp = _mm_load_ps(&pData[j].x);
		vTmp = _mm_mul_ps(vTmp, vToUINT8);

		//vx::storeFloat(&color, vTmp);
		_mm_store_ps(&pData[j].x, vTmp);

		pngData[i] = pData[j].x;
		pngData[i + 1] = pData[j].y;
		pngData[i + 2] = pData[j].z;
	}

	_aligned_free(pData);

	U8 *last_row = pngData.get() + (resolution.x * 3 * (resolution.y - 1));

	__time64_t long_time;
	_time64(&long_time);
	tm tm;
	localtime_s(&tm, &long_time);

	char nameBuffer[36];
	sprintf_s(nameBuffer, "screenshot %04i-%02i-%02i_%02i.%02i.%02i.png", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	stbi_write_png(nameBuffer, resolution.x, resolution.y, 3, last_row, -3 * resolution.x);
}

void RenderAspect::takeScreenshot()
{
	const auto resDim = m_pColdData->m_windowResolution.x * m_pColdData->m_windowResolution.y;
	const auto pixelBufferSizeBytes = sizeof(vx::float4) * resDim;

	m_pColdData->m_screenshotBuffer.bind();
	glReadPixels(0, 0, m_pColdData->m_windowResolution.x, m_pColdData->m_windowResolution.y, GL_RGBA, GL_FLOAT, 0);

	auto pScreenshotData = (vx::float4*)_aligned_malloc(pixelBufferSizeBytes, 16);
	auto p = m_pColdData->m_screenshotBuffer.map<U8>(vx::gl::Map::Read_Only);
	memcpy(pScreenshotData, p.get(), pixelBufferSizeBytes);
	p.unmap();

	struct Test
	{
		void operator()(const vx::uint2 &resolution, vx::float4 *pData)
		{
			auto size = resolution.x * resolution.y * 3;
			std::unique_ptr<U8[]> pngData = std::make_unique<U8[]>(size);

			const __m128 vToUINT8 = { 255.0f, 255.0f, 255.0f, 255.0f };
			//const __m128 invGamma = { 1.0f / 2.2f, 1.0f / 2.2f, 1.0f / 2.2f, 0.0f };

			for (U32 i = 0, j = 0; i < size; i += 3, ++j)
			{
				__m128 vTmp = _mm_load_ps(&pData[j].x);
				vTmp = _mm_mul_ps(vTmp, vToUINT8);

				//vx::storeFloat(&color, vTmp);
				_mm_store_ps(&pData[j].x, vTmp);

				pngData[i] = pData[j].x;
				pngData[i + 1] = pData[j].y;
				pngData[i + 2] = pData[j].z;
			}

			_aligned_free(pData);

			U8 *last_row = pngData.get() + (resolution.x * 3 * (resolution.y - 1));

			__time64_t long_time;
			_time64(&long_time);
			tm tm;
			localtime_s(&tm, &long_time);

			char nameBuffer[36];
			sprintf_s(nameBuffer, "screenshot %04i-%02i-%02i_%02i.%02i.%02i.png", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

			stbi_write_png(nameBuffer, resolution.x, resolution.y, 3, last_row, -3 * resolution.x);
		}
	};

	std::async(Test(), m_pColdData->m_windowResolution, pScreenshotData);
	//std::async(std::bind(writeScreenshot, this), );
}

void RenderAspect::handleEvent(const Event &evt)
{
	switch (evt.type)
	{
	case(EventType::File_Event) :
		handleFileEvent(evt);
		break;
	case(EventType::Ingame_Event) :
		handleIngameEvent(evt);
		break;
	default:
		break;
	}
}

void RenderAspect::handleFileEvent(const Event &evt)
{
	auto fileEvent = (FileEvent)evt.code;

	switch (fileEvent)
	{
	case FileEvent::Scene_Loaded:
	{
		auto pScene = (Scene*)evt.arg1.ptr;
		loadCurrentScene(pScene);
		m_pScene = pScene;
	}break;
	default:
		break;
	}
}

void RenderAspect::handleIngameEvent(const Event &evt)
{
	auto type = (IngameEvent)evt.code;

	if (type == IngameEvent::Created_NavGraph)
	{
		NavGraph* pGraph = (NavGraph*)evt.arg1.ptr;

		auto nodeCount = pGraph->getNodeCount();
		auto pNodes = pGraph->getNodes();

		auto ptrNodes = std::make_unique<vx::float3[]>(nodeCount);

		for (U32 i = 0; i < nodeCount; ++i)
		{
			auto &node = pNodes[i];
			ptrNodes[i] = node.m_position;
		}

		auto p = m_pColdData->m_navNodesVbo.map<vx::float3>(vx::gl::Map::Write_Only);
		vx::memcpy(p.get(), ptrNodes.get(), nodeCount);
		p.unmap();

		m_navNodesCount = nodeCount;

		auto connectionCount = pGraph->getConnectionCount();
		auto pConnections = pGraph->getConnections();

		auto connectionIndexCount = connectionCount * 2;
		VX_ASSERT(connectionIndexCount <= s_maxNavMeshIndices * 2);
		auto ptrConnections = std::make_unique<U16[]>(connectionIndexCount);

		U32 j = 0;
		for (U32 i = 0; i < connectionCount; ++i)
		{
			auto &connection = pConnections[i];

			ptrConnections[j++] = connection.m_fromNode;
			ptrConnections[j++] = connection.m_toNode;
		}

		VX_ASSERT(j == connectionIndexCount);

		auto pGpuConnections = m_pColdData->m_navConnectionsIbo.map<U16>(vx::gl::Map::Write_Only);
		vx::memcpy(pGpuConnections.get(), ptrConnections.get(), connectionIndexCount);
		pGpuConnections.unmap();

		m_navConnectionCount = connectionIndexCount;
	}
}

void RenderAspect::updateNavMeshBuffer(const NavMesh &navMesh)
{
	auto pVertices = navMesh.getVertices();
	auto pIndices = navMesh.getIndices();

	auto vertexCount = navMesh.getVertexCount();
	auto indexCount = navMesh.getIndexCount();

	auto p = m_pColdData->m_navmeshVbo.map<vx::float3>(vx::gl::Map::Write_Only);
	vx::memcpy(p.get(), pVertices, vertexCount);
	p.unmap();

	auto pi = m_pColdData->m_navmeshIbo.map<U16>(vx::gl::Map::Write_Only);
	vx::memcpy(pi.get(), pIndices, indexCount);
	pi.unmap();

	m_navmeshIndexCount = indexCount;
}

void RenderAspect::getProjectionMatrix(vx::mat4* m)
{
	*m = m_renderContext.getProjectionMatrix();
}