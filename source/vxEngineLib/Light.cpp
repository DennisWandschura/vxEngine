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
#include <vxEngineLib/Light.h>

void Light::getTransformationMatrix(vx::mat4* m) const
{
	const __m128 x_axis = { 1, 0, 0, 0 };
	const __m128 y_axis = { 0, 1, 0, 0 };

	auto lightPos = vx::loadFloat3(m_position);
	auto lightDir = vx::loadFloat3(m_direction);

	auto upDir = vx::cross3(x_axis, lightDir);
	vx::float4a dot = vx::dot3(upDir, upDir);
	if (dot.x == 0.0f)
	{
		upDir = vx::cross3(y_axis, lightDir);
	}

	auto projMatrix = vx::MatrixPerspectiveFovRH(vx::degToRad(m_angle), 1.0f, 0.1f, m_falloff);
	//auto projMatrix = vx::MatrixOrthographicRH(8, 5, 0.01f, m_falloff);
	auto viewMatrix = vx::MatrixLookToRH(lightPos, lightDir, upDir);

	*m = projMatrix * viewMatrix;
}