#include "Text.h"

Text::Text()
	:m_string(),
	m_layer(0)
{

}

Text::Text(const std::string &str, const vx::float2 &position, const vx::float2 &scale, const vx::float4 &color, F32 layer)
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

void Text::setPosition(F32 x, F32 y)
{
	m_position.x = x;
	m_position.y = y;
}

void Text::setPosition(const vx::float2 &position)
{
	m_position = position;
}

void Text::setColor(F32 r, F32 g, F32 b, F32 a)
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

void Text::setScale(F32 x, F32 y)
{
	m_scale.x = x;
	m_scale.y = y;
}

void Text::setScale(const vx::float2 &scale)
{
	m_scale = scale;
}