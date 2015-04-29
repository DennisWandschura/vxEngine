#pragma once

#include "Transform.h"

struct RenderUpdateTask
{
	enum class Type{ LoadScene, UpdateCamera, UpdateDynamicTransforms, TakeScreenshot, ToggleRenderMode, CreateActorGpuIndex };

	void* ptr;
	Type type;
};

struct RenderUpdateCameraData
{
	__m128 position;
	__m128 quaternionRotation;
};

struct RenderUpdateDataTransforms
{
	vx::TransformGpu* transforms;
	U32* indices;
	U32 count;
};