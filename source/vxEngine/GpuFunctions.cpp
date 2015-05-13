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
#include "GpuFunctions.h"
#include <stdint.h>

vx::uint2 __vectorcall GpuFunctions::packQRotation(const __m128 qRotation)
{
	const __m128 maxV = { 0xffff, 0xffff, 0xffff, 0xffff };

	auto qq = vx::fma(qRotation, vx::g_VXOneHalf, vx::g_VXOneHalf);
	qq = _mm_mul_ps(qq, maxV);

	vx::float4a tmp = qq;

	vx::uint2 packedQRotation;
	packedQRotation.x = (u32)tmp.x | (u32)tmp.y << 16;
	packedQRotation.y = (u32)tmp.z | (u32)tmp.w << 16;

	return packedQRotation;
}