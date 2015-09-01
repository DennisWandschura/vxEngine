#include "Frustum.h"
#include <vxEngineLib/Plane.h>

namespace FrustumCpp
{
	bool __vectorcall testFrustumPlane(__m128 c, __m128 r, const PlaneSIMD &p)
	{
		static const __m128 zero = { 0, 0, 0, 0 };
		auto dist = vx::dot3(c, p.n);
		dist = _mm_sub_ps(dist, p.d);
		dist = _mm_add_ps(dist, r);

		auto cmp = _mm_cmpge_ss(dist, zero);
		auto mask = _mm_movemask_ps(cmp);

		return (mask & (1 << 0));
	}
}

Frustum::Frustum()
{

}

Frustum::~Frustum()
{

}

void Frustum::update(const vx::mat4 &invPvMatrix)
{
	vx::float4a lbf = { -1, -1, 1, 1 };
	lbf = vx::Vector3TransformCoord(invPvMatrix, lbf);

	vx::float4a rbf = { 1, -1, 1, 1 };
	rbf = vx::Vector3TransformCoord(invPvMatrix, rbf);

	vx::float4a rtf = { 1, 1, 1, 1 };
	rtf = vx::Vector3TransformCoord(invPvMatrix, rtf);

	vx::float4a lbn = { -1, -1, 0, 1 };
	lbn = vx::Vector3TransformCoord(invPvMatrix, lbn);

	vx::float4a rtn = { 1, 1, 0, 1 };
	rtn = vx::Vector3TransformCoord(invPvMatrix, rtn);

	vx::float4a ltn = { -1, 1, 0, 1 };
	ltn = vx::Vector3TransformCoord(invPvMatrix, ltn);

	auto plane = PlaneSIMD::create(lbn, ltn, rtn);
	m_planes[FrustumPlaneNear].nd = plane.n;
	m_planes[FrustumPlaneNear].nd.m128_f32[3] = plane.d.m128_f32[0];

	plane = PlaneSIMD::create(lbn, lbf, ltn);
	m_planes[FrustumPlaneLeft].nd = plane.n;
	m_planes[FrustumPlaneLeft].nd.m128_f32[3] = plane.d.m128_f32[0];

	plane = PlaneSIMD::create(rbf, rtn, rtf);
	m_planes[FrustumPlaneRight].nd = plane.n;
	m_planes[FrustumPlaneRight].nd.m128_f32[3] = plane.d.m128_f32[0];
}

u32 __vectorcall Frustum::testSpheres(__m128* cr, u32 count, u32* indices) const
{
	PlaneSIMD planes[3];
	planes[0].d = VX_PERMUTE_PS(m_planes[0].nd, _MM_SHUFFLE(3, 3, 3, 3));
	planes[0].n = _mm_and_ps(m_planes[0].nd, vx::g_VXMask3);

	planes[1].d = VX_PERMUTE_PS(m_planes[1].nd, _MM_SHUFFLE(3, 3, 3, 3));
	planes[1].n = _mm_and_ps(m_planes[1].nd, vx::g_VXMask3);

	planes[2].d = VX_PERMUTE_PS(m_planes[2].nd, _MM_SHUFFLE(3, 3, 3, 3));
	planes[2].n = _mm_and_ps(m_planes[2].nd, vx::g_VXMask3);

	u32 visibileCount = 0;
	for (u32 i = 0; i < count; ++i)
	{
		auto bounds = cr[i];
		auto radius = VX_PERMUTE_PS(bounds, _MM_SHUFFLE(3, 3, 3, 3));
		auto center = _mm_and_ps(bounds, vx::g_VXMask3);

		u8 inside = 1;
		for (u32 j = 0; j < 3; ++j)
		{
			if (!FrustumCpp::testFrustumPlane(center, radius, planes[j]))
			{
				inside = 0;
				break;
			}
		}

		if (inside != 0)
		{
			indices[visibileCount++] = i;
		}
	}

	return visibileCount;
}