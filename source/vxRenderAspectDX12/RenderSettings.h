#pragma once

#include <vxLib/math/matrix.h>

struct RenderSettings
{
	vx::uint2 m_resolution;
	vx::mat4d m_projectionMatrix;
	vx::mat4d m_viewMatrixPrev;
	f64 m_nearZ;
	f64 m_farZ;
	f64 m_fovRad;
	u32 m_gpuLightCount;
	u32 m_textureDim;
};