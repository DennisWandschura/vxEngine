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