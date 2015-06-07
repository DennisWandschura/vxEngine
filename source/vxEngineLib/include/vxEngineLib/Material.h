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

#include <vxLib/types.h>
#include <vxLib/StringID.h>
#include <vxEngineLib/TextureRef.h>

class Material
{
	TextureRef m_albedo;
	TextureRef m_normal;
	TextureRef m_surface;
	f32 m_staticFriction{ 1.0f };
	f32 m_dynamicFriction{ 1.0f };
	f32 m_restitution{ 0 };

public:
	vx::StringID m_textureSid[3];

	Material();
	Material(const Material&) = delete;
	Material(Material &&rhs) noexcept;

	Material& operator=(const Material&) = delete;
	Material& operator=(Material &&rhs) noexcept;

	void swap(Material &rhs) noexcept;

	void setPhysx(f32 staticFriction, f32 dynamicFriction, f32 restitution);
	void setTextures(TextureRef &&albedo, TextureRef &&normal, TextureRef &&surface);

	const TextureRef& getAlbedoRef() const{ return m_albedo; }
	const TextureRef& getNormalRef() const{ return m_normal; }
	const TextureRef& getSurfaceRef() const{ return m_surface; }

	f32 getStaticFriction() const { return m_staticFriction; }
	f32 getDynamicFriction() const { return m_dynamicFriction; }
	f32 getRestitution() const { return m_restitution; }

	friend bool operator==(const Material&, const Material&)
	{
		return true;
	}

	friend bool operator!=(const Material &lhs, const Material &rhs)
	{
		return !(lhs == rhs);
	}

	friend bool operator<(const Material&, const Material&)
	{
		return true;
	}
};

struct MaterialFile
{
	struct Buffer
	{
		char data[32];
	};

	Buffer m_pipeline;
	Buffer m_albedo;
	Buffer m_normal;
	Buffer m_surface;
	f32 m_staticFriction{ 0 };
	f32 m_dynamicFriction{ 0 };
	f32 m_restitution{ 0 };

	void load(const u8 *ptr);
	bool loadFromFile(const char *file);
};