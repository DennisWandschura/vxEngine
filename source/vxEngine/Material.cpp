#include "Material.h"
#include <yaml-cpp\yaml.h>
#include "TextureFile.h"

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
	memcpy(m_textureSid, rhs.m_textureSid, sizeof(m_textureSid));
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

void Material::setPhysx(F32 staticFriction, F32 dynamicFriction, F32 restitution)
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

void MaterialFile::load(const U8 *ptr)
{
	memcpy(m_pipeline.data, ptr, sizeof(Buffer) * 4);
}

bool MaterialFile::loadFromFile(const char *file)
{
	try
	{
		YAML::Node root = YAML::LoadFile(file);

		auto strAlbedo = root["albedo"].as<std::string>();
		auto strNormals = root["normals"].as<std::string>();
		auto strSurface = root["surface"].as<std::string>();
		auto staticFriction = root["static friction"].as<F32>();
		auto dynamicFriction = root["dynamic friction"].as<F32>();
		auto restitution = root["restitution"].as<F32>();

		strncpy_s(m_albedo.data, strAlbedo.c_str(), strAlbedo.size());
		strncpy_s(m_normal.data, strNormals.c_str(), strNormals.size());
		strncpy_s(m_surface.data, strSurface.c_str(), strSurface.size());
		m_staticFriction = staticFriction;
		m_dynamicFriction = dynamicFriction;
		m_restitution = restitution;

	}
	catch (YAML::Exception &e)
	{
		VX_UNREFERENCED_PARAMETER(e);
		return false;
	}


	return true;
}