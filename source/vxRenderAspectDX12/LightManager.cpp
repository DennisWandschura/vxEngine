#include "LightManager.h"
#include "GpuLight.h"
#include <vxEngineLib/Graphics/Light.h>
#include "Frustum.h"
#include "GpuShadowTransform.h"
#include "ResourceManager.h"
#include "UploadManager.h"
#include <vxEngineLib/AABB.h>
#include "GpuVoxel.h"
#include "RenderPassLight.h"
#include "RenderAspect.h"
#include <vxEngineLib/Graphics/LightGeometryProxy.h>
#include <vxEngineLib/algorithm.h>

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
	m_proxyCount(0),
	m_visibleLightCount(0),
	m_sceneLightCount(0),
	m_maxSceneLightCount(0),
	m_maxShadowCastingLights(0),
	m_resultBufferCount(0)
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

void LightManager::getRequiredMemory(u64* heapSizeBuffere, u32* bufferCount, u32 maxSceneLightCount, u32 maxShadowCastingLights)
{
	const auto lightBufferSize = d3d::getAlignedSize(sizeof(GpuLight) * maxSceneLightCount, 64llu KBYTE);
	const auto visibleLightsBufferSize = d3d::getAlignedSize(sizeof(u32) * maxSceneLightCount, 64llu KBYTE);

	const u64 shadowTransformBufferSize = d3d::getAlignedSize(sizeof(GpuShadowTransform) * maxShadowCastingLights, 64llu KBYTE);
	const u64 shadowReverseTransformBufferSize = d3d::getAlignedSize(sizeof(GpuShadowTransformReverse) * maxShadowCastingLights, 64llu KBYTE);
	const auto shadowCastingLightsBufferSize = d3d::getAlignedSize(sizeof(GpuLight) * maxShadowCastingLights, 64llu KBYTE);

	const auto voxelBufferSize = d3d::getAlignedSize(sizeof(GpuVoxel), 64llu KBYTE);

	*heapSizeBuffere += shadowTransformBufferSize + lightBufferSize + visibleLightsBufferSize + shadowCastingLightsBufferSize + shadowReverseTransformBufferSize + voxelBufferSize;
	*bufferCount += 6;
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

	srvDesc.Buffer.StructureByteStride = sizeof(GpuShadowTransformReverse);
	resourceManager->insertShaderResourceView("shadowReverseTransformBufferView", srvDesc);
}

bool LightManager::initialize(const RenderSettings &settings, vx::StackAllocator* allocator, u32 maxSceneLightCount, d3d::ResourceManager* resourceManager, UploadManager* uploadManager)
{
	auto maxShadowCastingLights = settings.m_shadowCastingLightCount;

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

	const auto voxelBufferSize = d3d::getAlignedSize(sizeof(GpuVoxel), 64llu KBYTE);
	auto voxelBuffer = resourceManager->createBuffer(L"voxelBuffer", voxelBufferSize, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	if (voxelBuffer == nullptr)
		return false;

	createSrvLights(maxSceneLightCount, resourceManager);
	createSrvShadowCastingLights(maxShadowCastingLights, resourceManager);

	m_maxShadowCastingLights = maxShadowCastingLights;
	m_maxSceneLightCount = maxSceneLightCount;

	m_downloadEvent = Event::createEvent();
	m_downloadEvent.setStatus(EventStatus::Queued);

	m_checkLightsEvent = Event::createEvent();
	m_checkLightsEvent.setStatus(EventStatus::Complete);

	//m_renderPassCullLights->setEvent(m_checkLightsEvent, m_downloadEvent, (u8*)m_visibleLightsResult);

	auto gridsize = settings.m_lpvGridSize;
	auto dim = settings.m_lpvDim;

	auto halfDim = dim / 2;
	auto gridHalfSize = gridsize / 2.0f;

	auto gridCellSize = gridHalfSize / halfDim;
	auto invGridCellSize = 1.0f / gridCellSize;

	const __m128 axisY = { 0, 1, 0, 0 };
	const __m128 axisX = { 1, 0, 0, 0 };

	auto projectionMatrix = vx::MatrixOrthographicRHDX(gridsize, gridsize, 0.0f, gridsize);
	auto backFront = projectionMatrix * vx::MatrixTranslation(0, 0, -gridHalfSize);
	auto leftRight = projectionMatrix * vx::MatrixRotationAxis(axisY, vx::degToRad(90)) * vx::MatrixTranslation(gridHalfSize, 0, 0);
	auto upDown = projectionMatrix * vx::MatrixTranslation(0, 0, -gridHalfSize) * vx::MatrixRotationAxis(axisX, vx::degToRad(90));

	GpuVoxel voxelBufferData;
	voxelBufferData.projectionMatrix[0] = leftRight;
	voxelBufferData.projectionMatrix[1] = upDown;
	voxelBufferData.projectionMatrix[2] = backFront;
	voxelBufferData.dim = dim;
	voxelBufferData.halfDim = halfDim;
	voxelBufferData.gridCellSize = gridCellSize;
	voxelBufferData.invGridCellSize = invGridCellSize;

	uploadManager->pushUploadBuffer(voxelBufferData, voxelBuffer->get(), 0, voxelBuffer->getOriginalState());

	return true;
}

bool LightManager::loadSceneLights(const Graphics::Light* lights, u32 lightCount, Graphics::LightGeometryProxy* proxies, u32 proxyCount, ID3D12Device* device, d3d::ResourceManager* resourceManager, UploadManager* uploadManager)
{
	VX_ASSERT(lightCount <= m_maxSceneLightCount);

	for (u32 i = 0; i < lightCount; ++i)
	{
		m_sceneLights[i].position = vx::float4(lights[i].m_position, 1);
		m_sceneLights[i].falloff = lights[i].m_falloff;
		m_sceneLights[i].lumen = lights[i].m_lumen;

		LightManagerCpp::getShadowTransform(m_sceneLights[i], &m_sceneShadowTransforms[i], &m_sceneShadowReverseTransforms[i]);

		__m128 cr = m_sceneLights[i].position;
		cr.m128_f32[3] = lights[i].m_falloff;

		m_sceneLightBounds[i] = cr;
	}

	m_sceneLightCount = lightCount;

	m_proxies = std::make_unique<Graphics::LightGeometryProxy[]>(proxyCount);
	for (u32 i = 0; i < proxyCount; ++i)
	{
		m_proxies[i] = proxies[i];
	}
	m_proxyCount = proxyCount;

	return true;
}

void __vectorcall LightManager::update(__m128 cameraPosition, __m128 cameraDirection, const Frustum &frustum, d3d::ResourceManager* resourceManager, UploadManager* uploadManager, RenderAspect* renderAspect)
{
	auto sceneLightCount = m_sceneLightCount;
	if (sceneLightCount == 0)
		return;
	
	std::vector<Graphics::LightGeometryProxy*> containingProxies;
	auto proxyCount = m_proxyCount;
	for (u32 i = 0; i < proxyCount; ++i)
	{
		auto &proxy = m_proxies[i];
		if(proxy.m_bounds.contains(cameraPosition))
		{
			containingProxies.push_back(&proxy);
		}
	}

	std::vector<Graphics::LightGeometryProxy*> intersectingProxies;
	for (u32 i = 0; i < proxyCount; ++i)
	{
		auto &proxy = m_proxies[i];

		for (auto &it : containingProxies)
		{
			if (&proxy != it)
				if (proxy.m_bounds.intersects(it->m_bounds))
				{
					intersectingProxies.push_back(&proxy);
				}
		}
	}

	u32 totalLightCount = 0;
	for (auto &it : containingProxies)
	{
		totalLightCount += it->m_lightCount;
	}

	for (auto &it : intersectingProxies)
	{
		totalLightCount += it->m_lightCount;
	}
	std::vector<u16> lightIndies;
	lightIndies.reserve(totalLightCount);

	for (auto &it : containingProxies)
	{
		for (u32 i = 0; i < it->m_lightCount; ++i)
		{
			u16 lightIndex = it->m_lightIndices[i];
			auto it = vx::vector_find(lightIndies, lightIndex, std::less<u16>());
			if (it == lightIndies.end())
			{
				vx::vector_sortedInsert(&lightIndies, lightIndex, std::less<u16>());
			}
		}
	}

	for (auto &it : intersectingProxies)
	{
		for (u32 i = 0; i < it->m_lightCount; ++i)
		{
			u16 lightIndex = it->m_lightIndices[i];
			auto it = vx::vector_find(lightIndies, lightIndex, std::less<u16>());
			if (it == lightIndies.end())
			{
				vx::vector_sortedInsert(&lightIndies, lightIndex, std::less<u16>());
			}
		}
	}

	totalLightCount = lightIndies.size();
	VX_ASSERT(totalLightCount <= m_maxShadowCastingLights);
	auto marker = m_scratchAllocator.getMarker();

	auto shadowTransformsToGpu = (GpuShadowTransform*)m_scratchAllocator.allocate(sizeof(GpuShadowTransform) * totalLightCount, __alignof(GpuShadowTransform));
	auto shadowReverseTransformsToGpu = (GpuShadowTransformReverse*)m_scratchAllocator.allocate(sizeof(GpuShadowTransformReverse) * totalLightCount, __alignof(GpuShadowTransformReverse));
	auto lightsToGpu = (GpuLight*)m_scratchAllocator.allocate(sizeof(GpuLight) * totalLightCount, __alignof(GpuLight));
	for (u32 i = 0; i < totalLightCount; ++i)
	{
		auto index = lightIndies[i];

		lightsToGpu[i] = m_sceneLights[index];
		shadowTransformsToGpu[i] = m_sceneShadowTransforms[index];
		shadowReverseTransformsToGpu[i] = m_sceneShadowReverseTransforms[index];
	}

	auto shadowCastingLightsBuffer = resourceManager->getBuffer(L"shadowCastingLightsBuffer");
	auto shadowTransformBuffer = resourceManager->getBuffer(L"shadowTransformBuffer");
	auto shadowReverseTransformBuffer = resourceManager->getBuffer(L"shadowReverseTransformBuffer");

	auto sizeInBytesLights = sizeof(GpuLight) * totalLightCount;
	auto sizeInBytesShadow = sizeof(GpuShadowTransform) * totalLightCount;
	auto sizeInBytesShadowReverse = sizeof(GpuShadowTransformReverse) * totalLightCount;
	uploadManager->pushUploadBuffer((u8*)lightsToGpu, shadowCastingLightsBuffer->get(), 0, sizeInBytesLights, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	uploadManager->pushUploadBuffer((u8*)shadowTransformsToGpu, shadowTransformBuffer->get(), 0, sizeInBytesShadow, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	uploadManager->pushUploadBuffer((u8*)shadowReverseTransformsToGpu, shadowReverseTransformBuffer->get(), 0, sizeInBytesShadowReverse, shadowReverseTransformBuffer->getOriginalState());

	m_scratchAllocator.clear(marker);

	for (auto &it : m_renderPasses)
	{
		it->setVisibleLightCount(totalLightCount);
	}


	RenderUpdateTextData taskData;
	taskData.color = vx::float3(1, 1, 1);
	taskData.position = vx::float2(1, 1);
	taskData.strSize = snprintf(taskData.text, sizeof(taskData.text), "visible lights: %u", totalLightCount);
	renderAspect->queueUpdate(RenderUpdateTaskType::UpdateText, (u8*)&taskData, sizeof(taskData));
}

void LightManager::addStaticMeshInstance(const D3D12_DRAW_INDEXED_ARGUMENTS &cmd, const AABB &bounds, UploadManager* uploadManager)
{
}
