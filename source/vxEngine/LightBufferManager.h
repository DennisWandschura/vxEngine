#pragma once

struct Light;

#include <vxLib/gl/Buffer.h>

class LightBufferManager
{
	vx::gl::Buffer m_lightDataBuffer;
	U32 m_lightCount{ 0 };
	U32 m_maxLightCount{ 0 };

	void createLightDataBuffer(U32 maxLightCount);

public:
	void initialize(U32 maxLightCount);

	void bindBuffer();
	void updateLightDataBuffer(const Light* lights, U32 count);
};