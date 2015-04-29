#pragma once

struct RenderAspectDescription;
class SystemAspect;

namespace vx
{
	class Window;
	class StackAllocator;
}

#include <vxLib/math/Vector.h>

class EngineConfig
{
	vx::uint2 m_resolution{1920, 1080};
	U32 m_shadowMapResolution{2048};
	F32 m_fov{66.0f};
	F32 m_zNear{0.1f};
	F32 m_zFar{1000.0f};
	U8 m_voxelGiMode{0};
	bool m_vsync{false};
	bool m_renderDebug{false};

public:
	void loadFromYAML(const char* file);

	const vx::uint2& getResolution() const { return m_resolution; }
	bool isVSync() const { return m_vsync; }
	bool isRenderDebug() const { return m_renderDebug; }

	RenderAspectDescription getRenderAspectDescription(const vx::Window* window, vx::StackAllocator* allocator) const;
};