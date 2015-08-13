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
#include <vxEngineLib/Material.h>
#include <vxEngineLib/ParserNode.h>

/*Material::Material()
	:m_sid(),
	m_staticFriction(1.0f),
	m_dynamicFriction(1.0f),
	m_restitution(0.0f),
	m_textureSid()
{
}*/

Material::Material(const vx::StringID &sid)
	:m_sid(sid),
	m_staticFriction(1.0f),
	m_dynamicFriction(1.0f),
	m_restitution(0.0f),
	m_textureSid()
{

}

Material::Material(Material &&rhs) noexcept
	:m_sid(std::move(rhs.m_sid)),
	m_staticFriction(rhs.m_staticFriction),
	m_dynamicFriction(rhs.m_dynamicFriction),
	m_restitution(rhs.m_restitution)
{
	::memcpy(m_textureSid, rhs.m_textureSid, sizeof(m_textureSid));
}

Material& Material::operator=(Material &&rhs) noexcept
{
	if (this != &rhs)
	{
		this->swap(rhs);
	}
	return *this;
}

void Material::swap(Material &rhs) noexcept
{
	std::swap(m_sid, rhs.m_sid);
	std::swap(m_textureSid, rhs.m_textureSid);
	std::swap(m_staticFriction, rhs.m_staticFriction);
	std::swap(m_dynamicFriction, rhs.m_dynamicFriction);
	std::swap(m_restitution, rhs.m_restitution);
}

void Material::setPhysx(f32 staticFriction, f32 dynamicFriction, f32 restitution)
{
	m_staticFriction = staticFriction;
	m_dynamicFriction = dynamicFriction;
	m_restitution = restitution;
}
void MaterialFile::load(const u8 *ptr)
{
	memcpy(m_pipeline.data, ptr, sizeof(Buffer) * 4);
}

bool MaterialFile::loadFromFile(const char *file)
{
	Parser::Node root;
	root.createFromFile(file);

	std::string albedoTextureFile;
	root.get("albedo")->as(&albedoTextureFile);

	std::string normalTextureFile;
	root.get("normals")->as(&normalTextureFile);

	std::string surfaceTextureFile;
	root.get("surface")->as(&surfaceTextureFile);

	root.get("static friction")->as(&m_staticFriction);
	root.get("dynamic friction")->as(&m_dynamicFriction);
	root.get("restitution")->as(&m_restitution);

	strncpy(m_albedo.data, albedoTextureFile.c_str(), 32);
	strncpy(m_normal.data, normalTextureFile.c_str(), 32);
	strncpy(m_surface.data, surfaceTextureFile.c_str(), 32);

	return true;
}
