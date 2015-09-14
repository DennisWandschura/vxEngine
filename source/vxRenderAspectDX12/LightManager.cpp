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
#include <vxEngineLib/AABB.h>
#include "RenderPassShadow.h"
#include "RenderPassFilterRSM.h"

enum FrustumPlane { FrustumPlaneNear, FrustumPlaneLeft, FrustumPlaneRight };

namespace LightManagerCpp
{
	void getShadowTransform(const GpuLight &light, GpuShadowTransform* shadowTransform, GpuShadowTransformReverse* reverseTransform)
	{
		auto n = 0.1f;
		auto f = light.falloff;

		auto lightPos = light.position;
		auto projectionMatrix = vx::MatrixPerspectiveFovRHDX(vx::VX_PIDIV2, 1.0f, n, f);

		const __m128 upDirs[6] =
		{
			{ 0, 1, 0, 0 },
			{ 0, 1, 0, 0 },
			{ 0, 0, 1, 0 },
			{ 0, 0, -1, 0 },
			{ 0, 1, 0, 0 },
			{ 0, 1, 0, 0 }
		};

		const __m128 dirs[6] =
		{
			{ 1, 0, 0, 0 },
			{ -1, 0, 0, 0 },
			{ 0, 1, 0, 0 },
			{ 0, -1, 0, 0 },
			{ 0, 0, -1, 0 },
			{ 0, 0, 1, 0 }
		};

		for (u32 i = 0; i < 6; ++i)
		{
			auto viewMatrix = vx::MatrixLookToRH(lightPos, dirs[i], upDirs[i]);
			auto pvMatrix = projectionMatrix * viewMatrix;

			shadowTransform->pvMatrix[i] = pvMatrix;
			reverseTransform->invPvMatrix[i] = vx::MatrixInverse(pvMatrix);
		}
	}
}

LightManager::LightManager()
	:m_sceneLights(nullptr),
	m_sceneShadowTransforms(nullptr),
	m_gpuLights(nullptr),
	m_sceneLightCount(0),
	m_maxSceneLightCount(0),
	m_maxShadowCastingLights(0),
	m_resultBufferCount(0),
	m_renderPassShadow(nullptr),
	m_renderPassShading(nullptr),
	m_renderPassCullLights(nullptr),
	m_renderPassFilterRSM(nullptr)
{

}

LightManager::LightManager(LightManager &&rhs)
	:m_sceneLights(rhs.m_sceneLights),
	m_sceneShadowTransforms(rhs.m_sceneShadowTransforms),
	m_sceneLightBounds(rhs.m_sceneLightBounds),
	m_gpuLights(rhs.m_gpuLights),
	m_sceneLightCount(rhs.m_sceneLightCount),
	m_maxSceneLightCount(rhs.m_maxSceneLightCount),
	m_scratchAllocator(std::move(rhs.m_scratchAllocator))
{
	rhs.m_sceneLights = nullptr;
	rhs.m_sceneShadowTransforms = nullptr;
	rhs.m_sceneLightBounds = nullptr;
	rhs.m_gpuLights = nullptr;
	rhs.m_sceneLightCount = 0;
	rhs.m_maxSceneLightCount = 0;
}

LightManager::~LightManager()
{
	m_gpuLights = nullptr;
}

void LightManager::getRequiredMemory(u64* heapSizeBuffere, u32 maxSceneLightCount, u32 maxShadowCastingLights)
{
	const auto lightBufferSize = d3d::getAlignedSize(sizeof(GpuLight) * maxSceneLightCount, 64llu KBYTE);
	const auto visibleLightsBufferSize = d3d::getAlignedSize(sizeof(u32) * maxSceneLightCount, 64llu KBYTE);

	const u64 shadowTransformBufferSize = d3d::getAlignedSize(sizeof(GpuShadowTransform) * maxShadowCastingLights, 64llu KBYTE);
	const u64 shadowReverseTransformBufferSize = d3d::getAlignedSize(sizeof(GpuShadowTransformReverse) * maxShadowCastingLights, 64llu KBYTE);
	const auto shadowCastingLightsBufferSize = d3d::getAlignedSize(sizeof(GpuLight) * maxShadowCastingLights, 64llu KBYTE);

	const auto iblConstancBufferSize = 64 KBYTE;

	*heapSizeBuffere += shadowTransformBufferSize + lightBufferSize + visibleLightsBufferSize + shadowCastingLightsBufferSize + shadowReverseTransformBufferSize + iblConstancBufferSize;
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

void LightManager::createSrvShadowCastingLights(u32 maxCount, d3d::ResourceManager* resourceManager)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = maxCount;
	srvDesc.Buffer.StructureByteStride = sizeof(GpuLight);

	resourceManager->insertShaderResourceView("shadowCastingLightsBufferView", srvDesc);

	srvDesc.Buffer.StructureByteStride = sizeof(GpuShadowTransform);
	resourceManager->insertShaderResourceView("shadowTransformBufferView", srvDesc);
}

bool LightManager::initialize(vx::StackAllocator* allocator, u32 maxSceneLightCount, u32 maxShadowCastingLights, d3d::ResourceManager* resourceManager)
{
	m_sceneLights = (GpuLight*)allocator->allocate(sizeof(GpuLight) * maxSceneLightCount, 16);
	m_sceneShadowTransforms = (GpuShadowTransform*)allocator->allocate(sizeof(GpuShadowTransform) * maxSceneLightCount, __alignof(GpuShadowTransform));
	m_sceneShadowReverseTransforms = (GpuShadowTransformReverse*)allocator->allocate(sizeof(GpuShadowTransformReverse) * maxSceneLightCount, __alignof(GpuShadowTransformReverse));
	m_sceneLightBounds = (__m128*)allocator->allocate(sizeof(__m128) * maxSceneLightCount, 16);

	m_gpuLights = (GpuLight*)allocator->allocate(sizeof(GpuLight) * maxSceneLightCount, 16);
	m_gpuShadowTransforms = (GpuShadowTransform*)allocator->allocate(sizeof(GpuShadowTransform) * maxSceneLightCount, __alignof(GpuShadowTransform));
	m_gpuShadowReverseTransforms = (GpuShadowTransformReverse*)allocator->allocate(sizeof(GpuShadowTransformReverse) * maxSceneLightCount, __alignof(GpuShadowTransformReverse));

	m_visibleLightsResult = (u32*)allocator->allocate(sizeof(u32) * maxSceneLightCount, 4);

	auto scratchPtr = allocator->allocate(64 KBYTE, 16);
	m_scratchAllocator = vx::StackAllocator(scratchPtr, 64 KBYTE);

	const u64 shadowTransformBufferSize = d3d::getAlignedSize(sizeof(GpuShadowTransform) * maxShadowCastingLights, 64llu KBYTE);
	auto shadowTransformBuffer = resourceManager->createBuffer(L"shadowTransformBuffer", shadowTransformBufferSize, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	if (shadowTransformBuffer == nullptr)
		return false;

	const u64 shadowReverseTransformBufferSize = d3d::getAlignedSize(sizeof(GpuShadowTransformReverse) * maxShadowCastingLights, 64llu KBYTE);
	auto shadowReverseTransformBuffer = resourceManager->createBuffer(L"shadowReverseTransformBuffer", shadowReverseTransformBufferSize, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	if (shadowReverseTransformBuffer == nullptr)
		return false;

	const auto lightBufferSize = d3d::getAlignedSize(sizeof(GpuLight) * maxSceneLightCount, 64llu KBYTE);
	auto lightBuffer = resourceManager->createBuffer(L"lightBuffer", lightBufferSize, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	if (lightBuffer == nullptr)
		return false;

	const auto visibleLightsBufferSize = d3d::getAlignedSize(sizeof(u32) * maxSceneLightCount, 64llu KBYTE);
	auto visibleLightIndicesBuffer = resourceManager->createBuffer(L"visibleLightIndicesBuffer", visibleLightsBufferSize, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	if (visibleLightIndicesBuffer == nullptr)
		return false;

	const auto shadowCastingLightsBufferSize = d3d::getAlignedSize(sizeof(GpuLight) * maxShadowCastingLights, 64llu KBYTE);
	auto shadowCastingLightsBuffer = resourceManager->createBuffer(L"shadowCastingLightsBuffer", shadowCastingLightsBufferSize, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	if (shadowCastingLightsBuffer == nullptr)
		return false;

	const auto iblConstantBufferSize = 64 KBYTE;
	auto iblConstantBuffer = resourceManager->createBuffer(L"iblConstantBuffer", iblConstantBufferSize, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	if (iblConstantBuffer == nullptr)
		return false;

	createSrvLights(maxSceneLightCount, resourceManager);
	createSrvShadowCastingLights(maxShadowCastingLights, resourceManager);

	m_maxShadowCastingLights = maxShadowCastingLights;
	m_maxSceneLightCount = maxSceneLightCount;

	m_downloadEvent = Event::createEvent();
	m_downloadEvent.setStatus(EventStatus::Queued);

	m_checkLightsEvent = Event::createEvent();
	m_checkLightsEvent.setStatus(EventStatus::Complete);

	m_renderPassCullLights->setEvent(m_checkLightsEvent, m_downloadEvent, (u8*)m_visibleLightsResult);

	return true;
}

bool LightManager::loadSceneLights(const Light* lights, u32 count, ID3D12Device* device, d3d::ResourceManager* resourceManager, UploadManager* uploadManager)
{
	VX_ASSERT(count <= m_maxSceneLightCount);

	for (u32 i = 0; i < count; ++i)
	{
		m_sceneLights[i].position = vx::float4(lights[i].m_position, 1);
		m_sceneLights[i].falloff = lights[i].m_falloff;
		m_sceneLights[i].lumen = lights[i].m_lumen;

		LightManagerCpp::getShadowTransform(m_sceneLights[i], &m_sceneShadowTransforms[i], &m_sceneShadowReverseTransforms[i]);

		__m128 cr = m_sceneLights[i].position;
		cr.m128_f32[3] = lights[i].m_falloff;

		m_sceneLightBounds[i] = cr;
	}

	m_sceneLightCount = count;

	return true;
}

void __vectorcall LightManager::update(__m128 cameraPosition, __m128 cameraDirection, const Frustum &frustum, d3d::ResourceManager* resourceManager, UploadManager* uploadManager)
{
	struct Pair
	{
		f32 distance;
		u32 index;
	};

	//return;

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
			auto visibleTransforms = (GpuShadowTransform*)m_scratchAllocator.allocate(sizeof(GpuShadowTransform) * m_resultBufferCount, __alignof(GpuShadowTransform));
			auto visibleReverseTransforms = (GpuShadowTransformReverse*)m_scratchAllocator.allocate(sizeof(GpuShadowTransformReverse) * m_resultBufferCount, __alignof(GpuShadowTransformReverse));
			for (u32 i = 0; i < m_resultBufferCount; ++i)
			{
				auto visible = m_visibleLightsResult[i];
				if (visible != 0)
				{
					visibleTransforms[visibleLightCount] = m_gpuShadowTransforms[i];
					visibleReverseTransforms[visibleLightCount] = m_gpuShadowReverseTransforms[i];
					visibleLights[visibleLightCount++] = m_gpuLights[i];
				}
			}

			if (visibleLightCount != 0)
			{
				auto sortedPairs = (Pair*)m_scratchAllocator.allocate(sizeof(Pair) * visibleLightCount, __alignof(Pair));
				for (u32 i = 0; i < visibleLightCount; ++i)
				{
					auto lightPosition = visibleLights[i].position;
					auto distanceToLight = vx::length3(_mm_sub_ps(lightPosition, cameraPosition));

					_mm_store_ss(&sortedPairs[i].distance, distanceToLight);
					sortedPairs[i].index = i;
				}

				std::sort(sortedPairs, sortedPairs + visibleLightCount, [](const Pair &lhs, const Pair &rhs)
				{
					return lhs.distance < rhs.distance;
				});

				auto shadowTransformsToGpu = (GpuShadowTransform*)m_scratchAllocator.allocate(sizeof(GpuShadowTransform) * visibleLightCount, __alignof(GpuShadowTransform));
				auto shadowReverseTransformsToGpu = (GpuShadowTransformReverse*)m_scratchAllocator.allocate(sizeof(GpuShadowTransformReverse) * visibleLightCount, __alignof(GpuShadowTransformReverse));
				auto lightsToGpu = (GpuLight*)m_scratchAllocator.allocate(sizeof(GpuLight) * visibleLightCount, __alignof(GpuLight));
				for (u32 i = 0; i < visibleLightCount; ++i)
				{
					auto index = sortedPairs[i].index;

					lightsToGpu[i] = visibleLights[index];
					shadowTransformsToGpu[i] = visibleTransforms[index];
					shadowReverseTransformsToGpu[i] = visibleReverseTransforms[index];
				}

				auto shadowCastingLightsBuffer = resourceManager->getBuffer(L"shadowCastingLightsBuffer");
				auto shadowTransformBuffer = resourceManager->getBuffer(L"shadowTransformBuffer");
				auto shadowReverseTransformBuffer = resourceManager->getBuffer(L"shadowReverseTransformBuffer");

				auto sizeInBytesLights = sizeof(GpuLight) * visibleLightCount;
				auto sizeInBytesShadow = sizeof(GpuShadowTransform) * visibleLightCount;
				auto sizeInBytesShadowReverse = sizeof(GpuShadowTransformReverse) * visibleLightCount;
				uploadManager->pushUploadBuffer((u8*)lightsToGpu, shadowCastingLightsBuffer->get(), 0, sizeInBytesLights, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

				uploadManager->pushUploadBuffer((u8*)shadowTransformsToGpu, shadowTransformBuffer->get(), 0, sizeInBytesShadow, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

				uploadManager->pushUploadBuffer((u8*)shadowReverseTransformsToGpu, shadowReverseTransformBuffer->get(), 0, sizeInBytesShadowReverse, shadowReverseTransformBuffer->getOriginalState());

				m_renderPassShading->setLightCount(visibleLightCount);
				m_renderPassShadow->setLightCount(visibleLightCount);
				m_renderPassFilterRSM->setLightCount(visibleLightCount);
			}

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
			m_gpuShadowTransforms[i] = m_sceneShadowTransforms[index];
			m_gpuShadowReverseTransforms[i] = m_sceneShadowReverseTransforms[index];
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
}

void LightManager::setRenderPassCullLights(RenderPassCullLights* renderPassCullLights)
{
	m_renderPassCullLights = renderPassCullLights;
}