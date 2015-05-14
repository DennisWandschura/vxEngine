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
#include "Material.h"
#include "TextureFile.h"
#include "TextParserFile.h"

Material::Material()
	:m_albedo(),
	m_normal(),
	m_surface(),
	m_textureSid()
{
}

Material::Material(Material &&rhs) noexcept
	:m_albedo(std::move(rhs.m_albedo)),
	m_normal(std::move(rhs.m_normal)),
	m_surface(std::move(rhs.m_surface)),
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
	std::swap(m_albedo, rhs.m_albedo);
	std::swap(m_normal, rhs.m_normal);
	std::swap(m_surface, rhs.m_surface);
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

void Material::setTextures(TextureRef &&albedo, TextureRef &&normal, TextureRef &&surface)
{
	m_albedo = std::move(albedo);
	m_normal = std::move(normal);
	m_surface = std::move(surface);
}

void MaterialFile::load(const u8 *ptr)
{
	memcpy(m_pipeline.data, ptr, sizeof(Buffer) * 4);
}

bool MaterialFile::loadFromFile(const char *file)
{
	TextParser::File text;
	if (!text.createFromFile(file))
		return false;

	std::string albedoTextureFile;
	text.getNode("albedo")->as(albedoTextureFile);

	std::string normalTextureFile;
	text.getNode("normals")->as(normalTextureFile);

	std::string surfaceTextureFile;
	text.getNode("surface")->as(surfaceTextureFile);

	text.getNode("static friction")->as(m_staticFriction);
	text.getNode("dynamic friction")->as(m_dynamicFriction);
	text.getNode("restitution")->as(m_restitution);
	/*try
	{
		YAML::Node root = YAML::LoadFile(file);

		auto strAlbedo = root["albedo"].as<std::string>();
		auto strNormals = root["normals"].as<std::string>();
		auto strSurface = root["surface"].as<std::string>();
		auto staticFriction = root["static friction"].as<f32>();
		auto dynamicFriction = root["dynamic friction"].as<f32>();
		auto restitution = root["restitution"].as<f32>();

		strncpy(m_albedo.data, strAlbedo.c_str(), strAlbedo.size());
		strncpy(m_normal.data, strNormals.c_str(), strNormals.size());
		strncpy(m_surface.data, strSurface.c_str(), strSurface.size());
		m_staticFriction = staticFriction;
		m_dynamicFriction = dynamicFriction;
		m_restitution = restitution;

	}
	catch (YAML::Exception &e)
	{
		VX_UNREFERENCED_PARAMETER(e);
		return false;
	}
	*/

	strncpy(m_albedo.data, albedoTextureFile.c_str(), 32);
	strncpy(m_normal.data, normalTextureFile.c_str(), 32);
	strncpy(m_surface.data, surfaceTextureFile.c_str(), 32);

	return true;
}
