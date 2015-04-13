#include "GpuFunctions.h"
#include <stdint.h>

vx::uint2 __vectorcall GpuFunctions::packQRotation(const __m128 qRotation)
{
	const __m128 maxV = { 0xffff, 0xffff, 0xffff, 0xffff };

	auto qq = _mm_fmadd_ps(qRotation, vx::g_VXOneHalf, vx::g_VXOneHalf);
	qq = _mm_mul_ps(qq, maxV);

	vx::uint2 packedQRotation;
	packedQRotation.x = (U32)qq.m128_f32[0] | (U32)qq.m128_f32[1] << 16;
	packedQRotation.y = (U32)qq.m128_f32[2] | (U32)qq.m128_f32[3] << 16;

	return packedQRotation;
}