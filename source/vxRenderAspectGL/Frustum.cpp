#include "Frustum.h"
//#include <DirectXMath.h>

inline __m128 planeNormalize(__m128 P)
{
	// Perform the dot product on x,y and z only
	auto vLengthSq = _mm_mul_ps(P, P);
	auto vTemp = VX_PERMUTE_PS(vLengthSq, _MM_SHUFFLE(2, 1, 2, 1));
	vLengthSq = _mm_add_ss(vLengthSq, vTemp);
	vTemp = VX_PERMUTE_PS(vTemp, _MM_SHUFFLE(1, 1, 1, 1));
	vLengthSq = _mm_add_ss(vLengthSq, vTemp);
	vLengthSq = VX_PERMUTE_PS(vLengthSq, _MM_SHUFFLE(0, 0, 0, 0));
	// Prepare for the division
	auto vResult = _mm_sqrt_ps(vLengthSq);
	// Failsafe on zero (Or epsilon) length planes
	// If the length is infinity, set the elements to zero
	vLengthSq = _mm_cmpneq_ps(vLengthSq, vx::g_VXInfinity);
	// Reciprocal mul to perform the normalization
	vResult = _mm_div_ps(P, vResult);
	// Any that are infinity, set to zero
	vResult = _mm_and_ps(vResult, vLengthSq);
	return vResult;
}

void Frustum::update(const vx::mat4 &pvMatrix)
{
	auto matrix = vx::MatrixTranspose(pvMatrix);

	planes[FrustumPlanes::Left] = planeNormalize(_mm_add_ps(matrix.c[3], matrix.c[0]));
	//planes[FrustumPlanes::Left].m128_f32[3] = -planes[FrustumPlanes::Left].m128_f32[3];

	planes[FrustumPlanes::Right] = planeNormalize(_mm_sub_ps(matrix.c[3], matrix.c[0]));
	//planes[FrustumPlanes::Right].m128_f32[3] = -planes[FrustumPlanes::Right].m128_f32[3];

	planes[FrustumPlanes::Top] = planeNormalize(_mm_sub_ps(matrix.c[3], matrix.c[1]));
	//planes[FrustumPlanes::Top].m128_f32[3] = -planes[FrustumPlanes::Top].m128_f32[3];

	planes[FrustumPlanes::Bottom] = planeNormalize(_mm_add_ps(matrix.c[3], matrix.c[1]));
	planes[FrustumPlanes::Bottom].m128_f32[3] = -planes[FrustumPlanes::Bottom].m128_f32[3];
//
	//planes[FrustumPlanes::Near] = planeNormalize(_mm_add_ps(matrix.c[3], matrix.c[2]));
	planes[FrustumPlanes::Near] = planeNormalize(matrix.c[2]);
//	planes[FrustumPlanes::Near].m128_f32[3] = -planes[FrustumPlanes::Near].m128_f32[3];

	planes[FrustumPlanes::Far] = planeNormalize(_mm_sub_ps(matrix.c[3], matrix.c[2]));
	//planes[FrustumPlanes::Far].m128_f32[3] = -planes[FrustumPlanes::Far].m128_f32[3];
}

bool Frustum::intersects(__m128 center, __m128 radius) const
{
	auto negRadius = vx::negate(radius);
	for (u32 i = 0; i < 6; ++i)
	{
		auto d = VX_PERMUTE_PS(planes[i], _MM_SHUFFLE(3, 3, 3, 3));
		auto dist = _mm_add_ps(vx::dot3(planes[i], center), d);
		//if (dist < -radius)
		auto cmp = _mm_cmplt_ss(dist, negRadius);
		auto mask = _mm_movemask_ps(cmp);
		if(mask & 0x1)
		{
			return false;
		}
	}

	return true;
}