#pragma once

struct RenderUpdateTask
{
	enum class Type{ LoadScene, UpdateCamera, TakeScreenshot };

	void* ptr;
	Type type;
};

struct RenderUpdateCameraData
{
	__m128 position;
	__m128 quaternionRotation;
};