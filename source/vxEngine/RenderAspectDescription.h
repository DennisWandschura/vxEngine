#pragma once

namespace vx
{
	class Window;
	class StackAllocator;
}

#include <vxLib/math/Vector.h>

enum class VoxelGiMode : U8 {Normal = 0, High = 1};

struct RenderAspectDescription
{
	const vx::Window* window;
	vx::StackAllocator* pAllocator;
	vx::uint2 resolution;
	U32 shadowMapResolution;
	F32 fovRad;
	F32 z_near;
	F32 z_far;
	VoxelGiMode voxelGiMode;
	bool vsync;
	bool debug;
};