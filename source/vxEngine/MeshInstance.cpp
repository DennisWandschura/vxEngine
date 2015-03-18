#include "MeshInstance.h"
#include <cstring>
#include "yamlHelper.h"

MeshInstance::MeshInstance()
	:m_meshSid(),
	m_materialSid(),
	m_transform()
{
}

MeshInstance::MeshInstance(vx::StringID64 meshSid, vx::StringID64 materialSid, const vx::Transform &transform)
	:m_meshSid(meshSid),
	m_materialSid(materialSid),
	m_transform(transform)
{
}

#if _VX_EDITOR
void MeshInstance::setTranslation(const vx::float3 &translation)
{
	m_transform.m_translation = translation;
}
#endif

MeshInstanceFile::MeshInstanceFile()
	:m_mesh(),
	m_material(),
	m_transform()
{
}

MeshInstanceFile::MeshInstanceFile(const char(&meshName)[32], const char(&materialName)[32],
	const vx::Transform &transform)
	:m_mesh(),
	m_material(),
	m_transform(transform)
{
	strcpy_s(m_mesh, meshName);
	strcpy_s(m_material, materialName);
}

void MeshInstanceFile::load(const U8 *ptr)
{
	memcpy(this, ptr, sizeof(MeshInstanceFile));
}

void MeshInstanceFile::load(const YAML::Node &node)
{
	auto strMesh = node["mesh"].as<std::string>();
	auto strMaterial = node["material"].as<std::string>();
	m_transform.m_translation = node["translation"].as<vx::float3>();
	m_transform.m_rotation = node["rotation"].as<vx::float3>();
	m_transform.m_scaling = node["scaling"].as<F32>();

	strncpy_s(m_mesh, strMesh.data(), strMesh.size());
	strncpy_s(m_material, strMaterial.data(), strMaterial.size());
}

void MeshInstanceFile::save(YAML::Node &node) const
{
	node["mesh"] = std::string(m_mesh);
	node["material"] = std::string(m_material);
	node["translation"] = m_transform.m_translation;
	node["rotation"] = m_transform.m_rotation;
	node["scaling"] = m_transform.m_scaling;
}

const char* MeshInstanceFile::getMeshFile() const noexcept
{
	return m_mesh;
}

const char* MeshInstanceFile::getMaterialFile() const noexcept
{
	return m_material;
}