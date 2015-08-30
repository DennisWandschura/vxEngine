#pragma once

struct GpuLight;
struct Light;
class Frustum;
class RenderPassCullLights;

#include <vxLib/math/Vector.h>
#include <vxLib/Allocator/StackAllocator.h>

class LightManager
{
	GpuLight* m_sceneLights;
	__m128* m_sceneLightBounds;
	GpuLight* m_gpuLights;
	u32 m_sceneLightCount;
	u32 m_gpuLightCount;
	vx::StackAllocator m_scratchAllocator;
	RenderPassCullLights* m_renderPass;

public:
	LightManager();
	~LightManager();

	bool initialize(vx::StackAllocator* allocator, u32 gpuLightCount, RenderPassCullLights* renderPass);

	void loadSceneLights(const Light* lights, u32 count);

	void update(const Frustum &frustum);
};