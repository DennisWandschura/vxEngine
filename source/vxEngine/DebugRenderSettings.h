#pragma once

#include <vxLib/types.h>

enum class ShadingMode : U8 { Full, Albedo, Normals };

class DebugRenderSettings
{
	U8 m_voxelize{1};
	ShadingMode m_shading{ ShadingMode ::Full};

public:
	DebugRenderSettings() = default;

	void setShadingMode(ShadingMode mode) { m_shading = mode; }
	ShadingMode getShadingMode() const { return m_shading; }

	void setVoxelize(U8 voxelize){ m_voxelize = voxelize; }
	U8 voxelize() const { return m_voxelize; }
	void toggleVoxelize(){ m_voxelize = m_voxelize ^ 1; }
};