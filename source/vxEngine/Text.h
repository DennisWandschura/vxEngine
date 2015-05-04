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
#pragma once

#include <vxLib/math/Vector.h>
#include <string>

class Text
{
	std::string m_string;
	vx::float2 m_position{0, 0};
	F32 m_layer;
	vx::float2 m_scale{1, 1};
	vx::float4 m_color{1, 1, 1, 1};

public:
	Text();
	Text(const std::string &str, const vx::float2 &position, const vx::float2 &scale, const vx::float4 &color, F32 layer = 0.0f);

	void setString(const char *str);
	const std::string& getString() const { return m_string; }

	void setPosition(F32 x, F32 y);
	void setPosition(const vx::float2 &position);
	vx::float2 getPosition() const { return m_position; }

	void setColor(F32 r, F32 g, F32 b, F32 a);
	void setColor(const vx::float4 &color);
	const vx::float4& getColor() const { return m_color; }

	void setScale(F32 x, F32 y);
	void setScale(const vx::float2 &scale);
	vx::float2 getScale() const { return m_scale; }

	void setLayer(F32 layer){ m_layer = layer; }
	F32 getLayer() const { return m_layer; }
};