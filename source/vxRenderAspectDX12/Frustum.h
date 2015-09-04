#pragma once

#include <vxLib/math/matrix.h>
#include <vxEngineLib/Plane.h>

inline bool __vectorcall testFrustumPlane(__m128 c, __m128 r, const PlaneSIMD &p)
{
	static const __m128 zero = { 0, 0, 0, 0 };
	auto dist = vx::dot3(c, p.n);
	dist = _mm_sub_ps(dist, p.d);
	//dist = _mm_add_ps(dist, r);
	r.m128_f32[0] = -r.m128_f32[0];

	auto cmp = _mm_cmpge_ss(dist, r);
	auto mask = _mm_movemask_ps(cmp);

	return (mask & (1 << 0));
}

class Frustum
{
	struct PackedPlane
	{
		__m128 nd;
	};

	enum FrustumPlane { FrustumPlaneNear, FrustumPlaneLeft, FrustumPlaneRight, FrustumPlaneFar};

	PackedPlane m_planes[4];
	PackedPlane m_planeZ13;

public:
	Frustum();
	~Frustum();

	void update(const vx::mat4 &invPvMatrix);

	u32 __vectorcall testSpheres(__m128* cr, u32 count, u32* indices) const;

	void getPlaneZ13(PlaneSIMD* plane) const;
};