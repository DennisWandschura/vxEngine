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
#include "Text.h"

Text::Text()
	:m_string(),
	m_layer(0)
{

}

Text::Text(const std::string &str, const vx::float2 &position, const vx::float2 &scale, const vx::float4 &color, f32 layer)
	:m_string(str),
	m_position(position),
	m_layer(layer),
	m_scale(scale),
	m_color(color)
{

}

void Text::setString(const char *str)
{
	m_string = str;
}

void Text::setPosition(f32 x, f32 y)
{
	m_position.x = x;
	m_position.y = y;
}

void Text::setPosition(const vx::float2 &position)
{
	m_position = position;
}

void Text::setColor(f32 r, f32 g, f32 b, f32 a)
{
	m_color.x = r;
	m_color.y = g;
	m_color.z = b;
	m_color.w = a;
}

void Text::setColor(const vx::float4 &color)
{
	m_color = color;
}

void Text::setScale(f32 x, f32 y)
{
	m_scale.x = x;
	m_scale.y = y;
}

void Text::setScale(const vx::float2 &scale)
{
	m_scale = scale;
}