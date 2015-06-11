#pragma once
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

#include <vxLib/math/Vector.h>

namespace vx
{
	struct Transform;

	struct TransformOld
	{
		vx::float3 m_translation{ 0, 0, 0 };
		vx::float3 m_rotation{ 0, 0, 0 };
		f32 m_scaling{ 1.0f };

		TransformOld() = default;

		TransformOld(const float3 &t, const float3 &r, const f32 s)
			:m_translation(t), m_rotation(r), m_scaling(s){}

		void convertTo(Transform* transform);
	};

	struct Transform
	{
		vx::float3 m_translation{ 0, 0, 0 };
		vx::float4 m_qRotation{ 0, 0, 0, 1.0f };
		f32 m_scaling{ 1.0f };

		Transform() = default;

		Transform(const float3 &t, const float4 &qr, const f32 s)
			:m_translation(t), m_qRotation(qr), m_scaling(s){}
	};

	struct VX_ALIGN(16) TransformGpu
	{
		vx::float3 translation;
		f32 scaling;
		vx::uint2 packedQRotation;
	};
}