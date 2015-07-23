/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <vxEngineLib/Triangle.h>

f32 Triangle::getArea() const
{
	auto ab = m_points[1] - m_points[0];
	auto ac = m_points[2] - m_points[0];

	auto t = vx::cross(ab, ac);
	auto area = vx::length3(t) * 0.5f;

	return area;
}

vx::float3 Triangle::getCentroid() const
{
	return (m_points[0] + m_points[1] + m_points[2]) / 3.0f;
}

bool Triangle::contains(const vx::float3 &point) const
{
	auto a = vx::loadFloat3(m_points[0]);
	auto b = vx::loadFloat3(m_points[1]);
	auto c = vx::loadFloat3(m_points[2]);
	auto p = vx::loadFloat3(point);

	a = _mm_sub_ps(a, p);
	b = _mm_sub_ps(b, p);
	c = _mm_sub_ps(c, p);

	auto u = vx::cross3(b, c);
	auto v = vx::cross3(c, a);

	vx::float4a uDotV = vx::dot3(u, v);
	if (uDotV.x < 0.0f)
		return false;

	auto w = vx::cross3(a, b);

	vx::float4a uDotW = vx::dot3(u, w);
	if (uDotW.x < 0.0f)
		return false;

	return true;
}

bool Triangle::sharesEdge(const Triangle &other) const
{
	u8 sharedCount = 0;

	for (int i = 0; i < 3; ++i)
	{
		auto otherPoint = other.m_points[i];

		if (m_points[0].x == otherPoint.x &&
			m_points[0].y == otherPoint.y &&
			m_points[0].z == otherPoint.z)
			++sharedCount;
		else if (m_points[1].x == otherPoint.x &&
			m_points[1].y == otherPoint.y &&
			m_points[1].z == otherPoint.z)
			++sharedCount;
		else if (m_points[2].x == otherPoint.x &&
			m_points[2].y == otherPoint.y &&
			m_points[2].z == otherPoint.z)
			++sharedCount;
	}

	return sharedCount > 1;
}