#include "Triangle.h"

F32 Triangle::getArea() const
{
	auto ab = m_points[1] - m_points[0];
	auto ac = m_points[2] - m_points[0];

	auto t = vx::cross(ab, ac);
	auto area = vx::length(t) * 0.5f;

	return area;
}

bool Triangle::contains(const vx::float3 &point) const
{
	auto a = vx::loadFloat(m_points[0]);
	auto b = vx::loadFloat(m_points[1]);
	auto c = vx::loadFloat(m_points[2]);
	auto p = vx::loadFloat(point);

	a = _mm_sub_ps(a, p);
	b = _mm_sub_ps(b, p);
	c = _mm_sub_ps(c, p);

	auto u = vx::Vector3Cross(b, c);
	auto v = vx::Vector3Cross(c, a);

	auto uDotV = vx::dot(u, v);
	if (uDotV < 0.0f)
		return false;

	auto w = vx::Vector3Cross(a, b);

	auto uDotW = vx::dot(u, w);
	if (uDotW < 0.0f)
		return false;

	return true;
}