#include "LightManager.h"
#include "GpuLight.h"
#include <vxEngineLib/Light.h>
#include "Frustum.h"
#include "RenderPassCullLights.h"
#include "RenderPassVisibleLights.h"
#include "GpuShadowTransform.h"
#include "ResourceManager.h"
#include "RenderPassShading.h"
#include "UploadManager.h"

enum FrustumPlane { FrustumPlaneNear, FrustumPlaneLeft, FrustumPlaneRight };

namespace LightManagerCpp
{
	void getShadowTransform(const GpuLight &light, ShadowTransform* shadowTransform)
	{
		auto n = 0.1f;
		auto f = light.falloff;

		auto lightPos = light.position;
		auto projectionMatrix = vx::MatrixPerspectiveFovRHDX(vx::VX_PIDIV2, 1.0f, n, f);

		//f32 ss = projectionMatrix.c[0].m128_f32[0] / projectionMatrix.c[1].m128_f32[1];

		//auto projectionMatrixBiased = vx::MatrixPerspectiveFovRHDX(vx::VX_PIDIV2, 1.0f, n, f);
		//auto projectionMatrix = DirectX::XMMatrixPerspectiveFovRH(vx::VX_PIDIV2, 1.0f, n, f);

		/*const __m128 upDirs[6] =
		{
			{ 0, -1, 0, 0 },
			{ 0, -1, 0, 0 },
			{ 0, 0, 1, 0 },
			{ 0, 0, -1, 0 },
			{ 0, -1, 0, 0 },
			{ 0, -1, 0, 0 }
		};*/

		const __m128 upDirs[6] =
		{
			{ 0, 1, 0, 0 },
			{ 0, 1, 0, 0 },
			{ 0, 0, -1, 0 },
			{ 0, 0, 1, 0 },
			{ 0, 1, 0, 0 },
			{ 0, 1, 0, 0 }
		};

		const __m128 dirs[6] =
		{
			{ 1, 0, 0, 0 },
			{ -1, 0, 0, 0 },
			{ 0, 1, 0, 0 },
			{ 0, -1, 0, 0 },
			{ 0, 0, 1, 0 },
			{ 0, 0, -1, 0 }
		};

		shadowTransform->projectionMatrix = projectionMatrix;
		for (u32 i = 0; i < 6; ++i)
		{
			auto viewMatrix = vx::MatrixLookToRH(lightPos, dirs[i], upDirs[i]);
			//auto viewMatrix = DirectX::XMMatrixLookToRH(lightPos, dirs[i], upDirs[i]);
			//shadowTransform->viewMatrix[i] = viewMatrix;
			shadowTransform->pvMatrix[i] = projectionMatrix * viewMatrix;
		}
	}
}

LightManager::LightManager()
	:m_sceneLights(nullptr),
	m_sceneShadowTransforms(nullptr),
	m_gpuLights(nullptr),
	m_sceneLightCount(0),
	m_maxLightCount(0),
	m_maxShadowCastingLights(0),
	m_resultBufferCount(0),
	m_renderPassShadow(nullptr),
	m_renderPassShading(nullptr),
	m_renderPassCullLights(nullptr)
{

}

LightManager::LightManager(LightManager &&rhs)
	:m_sceneLights(rhs.m_sceneLights),
	m_sceneShadowTransforms(rhs.m_sceneShadowTransforms),
	m_sceneLightBounds(rhs.m_sceneLightBounds),
	m_gpuLights(rhs.m_gpuLights),
	m_sceneLightCount(rhs.m_sceneLightCount),
	m_maxLightCount(rhs.m_maxLightCount),
	m_drawCommand(std::move(rhs.m_drawCommand)),
	m_scratchAllocator(std::move(rhs.m_scratchAllocator))
{
	rhs.m_sceneLights = nullptr;
	rhs.m_sceneShadowTransforms = nullptr;
	rhs.m_sceneLightBounds = nullptr;
	rhs.m_gpuLights = nullptr;
	rhs.m_sceneLightCount = 0;
	rhs.m_maxLightCount = 0;
}

LightManager::~LightManager()
{
	m_gpuLights = nullptr;
}

void LightManager::getRequiredMemory(u64* heapSizeBuffere, u32 maxLightCount, u32 maxShadowCastingLights)
{
	m_drawCommand.getRequiredMemory(64, heapSizeBuffere);

	const auto lightBufferSize = d3d::getAlignedSize(sizeof(GpuLight) * maxLightCount, 64llu KBYTE);
	const u64 shadowTransformBufferSize = d3d::getAlignedSize(sizeof(ShadowTransform) * maxShadowCastingLights, 64llu KBYTE);
	const auto visibleLightsBufferSize = d3d::getAlignedSize(sizeof(u32) * maxLightCount, 64llu KBYTE);
	const auto shadowCastingLightsBufferSize = d3d::getAlignedSize(sizeof(GpuLight) * maxShadowCastingLights, 64llu KBYTE);

	*heapSizeBuffere += shadowTransformBufferSize + lightBufferSize + visibleLightsBufferSize + shadowCastingLightsBufferSize;
}

void LightManager::createSrvLights(u32 maxCount, d3d::ResourceManager* resourceManager)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = maxCount;
	srvDesc.Buffer.StructureByteStride = sizeof(GpuLight);

	resourceManager->insertShaderResourceView("lightBufferView", srvDesc);
}

bool LightManager::initialize(vx::StackAllocator* allocator, u32 maxLightCount, u32 maxShadowCastingLights, d3d::ResourceManager* resourceManager)
{
	m_gpuLights = (GpuLight*)allocator->allocate(sizeof(GpuLight) * maxLightCount, 16);
	m_sceneLights = (GpuLight*)allocator->allocate(sizeof(GpuLight) * maxLightCount, 16);
	m_sceneShadowTransforms = (ShadowTransform*)allocator->allocate(sizeof(ShadowTransform) * maxLightCount, 16);
	m_sceneLightBounds = (__m128*)allocator->allocate(sizeof(__m128) * maxLightCount, 16);

	m_visibleLightsResult = (u32*)allocator->allocate(sizeof(u32) * maxLightCount, 4);

	auto scratchPtr = allocator->allocate(64 KBYTE, 16);
	m_scratchAllocator = vx::StackAllocator(scratchPtr, 64 KBYTE);

	const u64 shadowTransformBufferSize = d3d::getAlignedSize(sizeof(ShadowTransform) * maxShadowCastingLights, 64llu KBYTE);
	auto shadowTransformBuffer = resourceManager->createBuffer(L"shadowTransformBuffer", shadowTransformBufferSize, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	if (shadowTransformBuffer == nullptr)
		return false;

	const auto lightBufferSize = d3d::getAlignedSize(sizeof(GpuLight) * maxLightCount, 64llu KBYTE);
	auto lightBuffer = resourceManager->createBuffer(L"lightBuffer", lightBufferSize, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	if (lightBuffer == nullptr)
		return false;

	const auto visibleLightsBufferSize = d3d::getAlignedSize(sizeof(u32) * maxLightCount, 64llu KBYTE);
	auto visibleLightIndicesBuffer = resourceManager->createBuffer(L"visibleLightIndicesBuffer", visibleLightsBufferSize, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	if (visibleLightIndicesBuffer == nullptr)
		return false;

	const auto shadowCastingLightsBufferSize = d3d::getAlignedSize(sizeof(GpuLight) * maxShadowCastingLights, 64llu KBYTE);
	auto shadowCastingLightsBuffer = resourceManager->createBuffer(L"shadowCastingLightsBuffer", shadowCastingLightsBufferSize, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	if (shadowCastingLightsBuffer == nullptr)
		return false;

	createSrvLights(maxLightCount, resourceManager);

	m_maxShadowCastingLights = maxShadowCastingLights;
	m_maxLightCount = maxLightCount;

	m_downloadEvent = Event::createEvent();
	m_downloadEvent.setStatus(EventStatus::Queued);

	m_checkLightsEvent = Event::createEvent();
	m_checkLightsEvent.setStatus(EventStatus::Complete);

	m_renderPassCullLights->setEvent(m_checkLightsEvent, m_downloadEvent, (u8*)m_visibleLightsResult);

	return true;
}

bool LightManager::loadSceneLights(const Light* lights, u32 count, ID3D12Device* device, d3d::ResourceManager* resourceManager, UploadManager* uploadManager)
{
	VX_ASSERT(count <= m_maxLightCount);

	for (u32 i = 0; i < count; ++i)
	{
		m_sceneLights[i].position = vx::float4(lights[i].m_position, 1);
		m_sceneLights[i].falloff = lights[i].m_falloff;
		m_sceneLights[i].lumen = lights[i].m_lumen;

		LightManagerCpp::getShadowTransform(m_sceneLights[i], &m_sceneShadowTransforms[i]);

		__m128 cr = m_sceneLights[i].position;
		cr.m128_f32[3] = lights[i].m_falloff;

		m_sceneLightBounds[i] = cr;
	}

	m_sceneLightCount = count;

	if (!m_drawCommand.create(L"drawCmdShadow", 64, resourceManager, device))
		return false;

	auto shadowTransformBuffer = resourceManager->getBuffer(L"shadowTransformBuffer");
	uploadManager->pushUploadBuffer((u8*)&m_sceneShadowTransforms[0], shadowTransformBuffer->get(), 0, sizeof(ShadowTransform), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	auto shadowCastingLightsBuffer = resourceManager->getBuffer(L"shadowCastingLightsBuffer");
	auto sizeInBytes = sizeof(GpuLight) * count;
	uploadManager->pushUploadBuffer((u8*)m_sceneLights, shadowCastingLightsBuffer->get(), 0, sizeInBytes, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	m_renderPassShading->setLightCount(count);

	return true;
}

void __vectorcall LightManager::update(__m128 cameraPosition, __m128 cameraDirection, const Frustum &frustum, d3d::ResourceManager* resourceManager, UploadManager* uploadManager)
{
	struct Pair
	{
		f32 distance;
		u32 index;
	};

	auto sceneLightCount = m_sceneLightCount;
	if (sceneLightCount == 0)
		return;

	bool startCheck = false;

	auto statusCheckLights = m_checkLightsEvent.getStatus();
	if (statusCheckLights == EventStatus::Complete)
	{
		// checked on gpu

		auto dlStatus = m_downloadEvent.getStatus();
		if (dlStatus == EventStatus::Complete)
		{
			u32 visibleLightCount = 0;
			// finished dl to cpu, upload resulting light buffer to gpu

			auto marker = m_scratchAllocator.getMarker();
			auto visibleLights = (GpuLight*)m_scratchAllocator.allocate(sizeof(GpuLight) * m_resultBufferCount, __alignof(GpuLight));
			for (u32 i = 0; i < m_resultBufferCount; ++i)
			{
				auto visible = m_visibleLightsResult[i];
				if (visible != 0)
				{
					visibleLights[visibleLightCount++] = m_gpuLights[i];
				}
			}

			auto shadowCastingLightsBuffer = resourceManager->getBuffer(L"shadowCastingLightsBuffer");
			auto sizeInBytes = sizeof(GpuLight) * visibleLightCount;
			uploadManager->pushUploadBuffer((u8*)visibleLights, shadowCastingLightsBuffer->get(), 0, sizeInBytes, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			m_renderPassShading->setLightCount(visibleLightCount);

			m_scratchAllocator.clear(marker);

			m_resultBufferCount = 0;
			m_downloadEvent.setStatus(EventStatus::Queued);
		}
		else if(dlStatus == EventStatus::Running)
		{
			// wait
		}
		else
		{
			// start update
			startCheck = true;
		}
	}

	if (startCheck)
	{
		auto marker = m_scratchAllocator.getMarker();
		u32* indices = (u32*)m_scratchAllocator.allocate(sceneLightCount * sizeof(u32));
		u32 lightsInFrustumCount = frustum.testSpheres(m_sceneLightBounds, sceneLightCount, indices);

		__m128* boundsInFrustum = (__m128*)m_scratchAllocator.allocate(lightsInFrustumCount * sizeof(__m128), 16);
		for (u32 i = 0; i < lightsInFrustumCount; ++i)
		{
			auto index = indices[i];
			boundsInFrustum[i] = m_sceneLightBounds[index];
		}

		for (u32 i = 0; i < lightsInFrustumCount; ++i)
		{
			auto index = indices[i];

			m_gpuLights[i] = m_sceneLights[index];
		}

		auto lightBuffer = resourceManager->getBuffer(L"lightBuffer");
		auto sizeInBytes = sizeof(GpuLight) * lightsInFrustumCount;
		uploadManager->pushUploadBuffer((u8*)m_gpuLights, lightBuffer->get(), 0, sizeInBytes, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		m_checkLightsEvent.setStatus(EventStatus::Running);

		m_renderPassCullLights->setLightCount(lightsInFrustumCount);

		m_resultBufferCount = lightsInFrustumCount;
		m_scratchAllocator.clear(marker);
	}
}

void LightManager::addStaticMeshInstance(const D3D12_DRAW_INDEXED_ARGUMENTS &cmd, const AABB &bounds, UploadManager* uploadManager)
{
	auto &lightBounds = m_sceneLightBounds[0];

	vx::float3 p = vx::float3(lightBounds.m128_f32[0], lightBounds.m128_f32[1], lightBounds.m128_f32[2]);
	f32 r = lightBounds.m128_f32[3];

	auto vmin = vx::min(p, bounds.min);
	auto vmax = vx::max(p, bounds.max);

	vmin = bounds.min - vmin;
	vmax = vmax - bounds.max;

	f32 sqDist = vx::dot3(vmin, vmin);
	sqDist += vx::dot3(vmax, vmax);

	if (sqDist <= r * r)
	{
		auto count = m_drawCommand.getCount();

		m_drawCommand.uploadDrawCommand(count, cmd, uploadManager);
		m_drawCommand.setCount(count + 1, uploadManager);
	}
}

DrawIndexedIndirectCommand* LightManager::getDrawCommand(u32 i)
{
	return &m_drawCommand;
}

void LightManager::setRenderPassCullLights(RenderPassCullLights* renderPassCullLights)
{
	m_renderPassCullLights = renderPassCullLights;
}