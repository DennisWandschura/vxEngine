#include "LightManager.h"
#include "GpuLight.h"
#include <vxEngineLib/Light.h>
#include "Frustum.h"
#include "RenderPassCullLights.h"
#include "RenderPassVisibleLights.h"
#include "GpuShadowTransform.h"
#include "ResourceManager.h"
#include "UploadManager.h"

enum FrustumPlane { FrustumPlaneNear, FrustumPlaneLeft, FrustumPlaneRight };

namespace LightManagerCpp
{
	const u32 g_maxLights = 256;

	void getShadowTransform(const GpuLight &light, ShadowTransform* shadowTransform)
	{
		auto n = 0.1f;
		auto f = light.falloff;

		auto lightPos = light.position;
		auto projectionMatrix = vx::MatrixPerspectiveFovRHDX(vx::VX_PIDIV2, 1.0f, n, f);

		f32 ss = projectionMatrix.c[0].m128_f32[0] / projectionMatrix.c[1].m128_f32[1];

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
	m_gpuLightCount(0)
{

}

LightManager::~LightManager()
{
	m_gpuLights = nullptr;
}

void LightManager::getRequiredMemory(u64* heapSizeBuffere)
{
	m_drawCommand.getRequiredMemory(64, heapSizeBuffere);
	*heapSizeBuffere += 64llu KBYTE;
}

bool LightManager::initialize(vx::StackAllocator* allocator, u32 gpuLightCount, d3d::ResourceManager* resourceManager)
{
	//m_gpuLights = (GpuLight*)allocator->allocate(sizeof(GpuLight) * gpuLightCount, 16);
	m_gpuLightCount = gpuLightCount;

	m_sceneLights = (GpuLight*)allocator->allocate(sizeof(GpuLight) * LightManagerCpp::g_maxLights, 16);
	m_sceneShadowTransforms = (ShadowTransform*)allocator->allocate(sizeof(ShadowTransform) * LightManagerCpp::g_maxLights, 16);
	m_sceneLightBounds = (__m128*)allocator->allocate(sizeof(__m128) * LightManagerCpp::g_maxLights, 16);

	auto scratchPtr = allocator->allocate(64 KBYTE, 16);
	m_scratchAllocator = vx::StackAllocator(scratchPtr, 64 KBYTE);

	auto shadowTransformBuffer = resourceManager->createBuffer(L"shadowTransformBuffer", 64 KBYTE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	if (shadowTransformBuffer == nullptr)
		return false;

	return true;
}

bool LightManager::loadSceneLights(const Light* lights, u32 count, ID3D12Device* device, d3d::ResourceManager* resourceManager, UploadManager* uploadManager)
{
	VX_ASSERT(count <= LightManagerCpp::g_maxLights);

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
	m_renderPass->setLightCount(count);
	m_renderPassCopy->setLightCount(count);

	if (!m_drawCommand.create(L"drawCmdShadow", 64, resourceManager, device))
		return false;

	auto shadowTransformBuffer = resourceManager->getBuffer(L"shadowTransformBuffer");
	uploadManager->pushUploadBuffer((u8*)&m_sceneShadowTransforms[0], shadowTransformBuffer, 0, sizeof(ShadowTransform), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	return true;
}

void __vectorcall LightManager::update(__m128 cameraPosition, __m128 cameraDirection, const Frustum &frustum)
{
	struct Pair
	{
		f32 distance;
		u32 index;
	};

	auto sceneLightCount = m_sceneLightCount;
	if (sceneLightCount != 0)
	{
		auto marker = m_scratchAllocator.getMarker();
		u32* indices = (u32*)m_scratchAllocator.allocate(sceneLightCount * sizeof(u32));
		u32 count = frustum.testSpheres(m_sceneLightBounds, sceneLightCount, indices);

		__m128* boundsInFrustum = (__m128*)m_scratchAllocator.allocate(count * sizeof(__m128), 16);
		for (u32 i = 0; i < count; ++i)
		{
			auto index = indices[i];
			boundsInFrustum[i] = m_sceneLightBounds[index];
		}

		PlaneSIMD halfPlane;
		frustum.getPlaneZ13(&halfPlane);
		u32 nearHalfCount = 0;

		for (u32 i = 0; i < count; ++i)
		{
			auto bounds = boundsInFrustum[i];

			auto radius = VX_PERMUTE_PS(bounds, _MM_SHUFFLE(3, 3, 3, 3));
			auto center = _mm_and_ps(bounds, vx::g_VXMask3);

			if (testFrustumPlane(center,radius , halfPlane))
			{
				++nearHalfCount;
			}
		}

		/*Pair* pairs = (Pair*)m_scratchAllocator.allocate(count * sizeof(Pair), 16);
		for (u32 i = 0; i < count; ++i)
		{
			auto lightPositon = bounds[i];

			auto dirToLight = _mm_sub_ps(lightPositon, cameraPosition);
			auto distance = _mm_sqrt_ps(vx::dot3(dirToLight, dirToLight));
			dirToLight = _mm_div_ps(dirToLight, distance);

			auto dd = vx::dot3(dirToLight, cameraDirection);
		//	dirToLight = _mm_and_ps(dirToLight, vx::g_VXMask3);

			pairs[i].distance = distance.m128_f32[0];
			pairs[i].index = i;
		}*/

		m_scratchAllocator.clear(marker);
	}
}

void LightManager::setRenderPasses(RenderPassCullLights* renderPass, RenderPassVisibleLights* renderPassCopy)
{
	m_renderPass = renderPass;
	m_renderPassCopy = renderPassCopy;
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