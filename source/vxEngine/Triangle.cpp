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

	auto u = vx::cross3(b, c);
	auto v = vx::cross3(c, a);

	auto uDotV = vx::dot(u, v);
	if (uDotV < 0.0f)
		return false;

	auto w = vx::cross3(a, b);

	auto uDotW = vx::dot(u, w);
	if (uDotW < 0.0f)
		return false;

	return true;
}