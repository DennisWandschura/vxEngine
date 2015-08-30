#include "LightManager.h"
#include "GpuLight.h"
#include <vxEngineLib/Light.h>
#include "Frustum.h"
#include "RenderPassCullLights.h"

enum FrustumPlane { FrustumPlaneNear, FrustumPlaneLeft, FrustumPlaneRight };

struct Plane
{
	__m128 n;
	__m128 d;

	static Plane __vectorcall create(__m128 a, __m128 b, __m128 c)
	{
		auto tmp0 = _mm_sub_ps(b, a);
		auto tmp1 = _mm_sub_ps(c, a);

		Plane result;
		result.n = vx::normalize3(vx::cross3(tmp0, tmp1));
		result.d = vx::dot3(result.n, a);

		return result;
	}
};

bool __vectorcall testFrustumPlane(__m128 c, __m128 r, const Plane &p)
{
	static const __m128 zero = {0, 0, 0, 0};
	auto dist = vx::dot3(c, p.n);
	dist = _mm_sub_ps(dist, p.d);
	dist = _mm_add_ps(dist, r);

	/*if (dist.m128_f32[0] >= 0)
	{
		++count;
	}
	*/

	auto cmp = _mm_cmpge_ss(dist, zero);
	auto mask = _mm_movemask_ps(cmp);

	return (mask & (1 << 0));
}

namespace LightManagerCpp
{
	const u32 g_maxLights = 256;
}

LightManager::LightManager()
	:m_sceneLights(nullptr),
	m_gpuLights(nullptr),
	m_sceneLightCount(0),
	m_gpuLightCount(0)
{

}

LightManager::~LightManager()
{
	m_gpuLights = nullptr;
}

bool LightManager::initialize(vx::StackAllocator* allocator, u32 gpuLightCount, RenderPassCullLights* renderPass)
{
	m_renderPass = renderPass;
	//m_gpuLights = (GpuLight*)allocator->allocate(sizeof(GpuLight) * gpuLightCount, 16);
	//m_gpuLightCount = 0;

	//m_sceneLights = (GpuLight*)allocator->allocate(sizeof(GpuLight) * LightManagerCpp::g_maxLights, 16);
	//m_sceneLightBounds = (__m128*)allocator->allocate(sizeof(__m128) * LightManagerCpp::g_maxLights, 16);

	/*auto scratchPtr = allocator->allocate(64 KBYTE, 16);
	m_scratchAllocator = vx::StackAllocator(scratchPtr, 64 KBYTE);*/

	return true;
}

void LightManager::loadSceneLights(const Light* lights, u32 count)
{
	VX_ASSERT(count <= LightManagerCpp::g_maxLights);

	/*for (u32 i = 0; i < count; ++i)
	{
		m_sceneLights[i].position = vx::float4(lights[i].m_position, 1);
		m_sceneLights[i].falloff = lights[i].m_falloff;
		m_sceneLights[i].lumen = lights[i].m_lumen;

		__m128 lightPosition = m_sceneLights[i].position;

		__m128 cr = lightPosition;
		cr.m128_f32[3] = lights[i].m_falloff;

		m_sceneLightBounds[i] = cr;
	}

	m_sceneLightCount = count;*/
	m_renderPass->setLightCount(count);
}

void LightManager::update(const Frustum &frustum)
{
	/*auto sceneLightCount = m_sceneLightCount;
	if (sceneLightCount != 0)
	{
		auto marker = m_scratchAllocator.getMarker();
		u8 * results = (u8*)m_scratchAllocator.allocate(sceneLightCount);

		frustum.testSpheres(m_sceneLightBounds, sceneLightCount, results);
		u32 count = 0;
		for (u32 i = 0; i < sceneLightCount; ++i)
		{
			if (results[i] != 0)
			{
				++count;
			}
		}

		m_scratchAllocator.clear(marker);
	}*/
}