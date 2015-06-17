#pragma once

#include <vxLib/math/matrix.h>

typedef u32 uint;

typedef vx::mat4 mat4;

typedef vx::uint2a uvec2;

typedef vx::float2a vec2;
typedef __m128 vec4;

typedef u64 sampler2D;
typedef u64 sampler2DArray;
typedef u64 sampler3D;

typedef u64 samplerCube;
typedef u64 samplerCubeShadow;

typedef u64 image2D;
typedef u64 uimage3D;

#define UNIFORM(NAME, X) struct NAME
#define UNIFORM_END(NAME) ;
#define PARAMS(P) 