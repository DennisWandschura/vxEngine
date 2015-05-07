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
#include "MeshInstance.h"
#include <cstring>

MeshInstance::MeshInstance()
	:m_meshSid(),
	m_materialSid(),
	m_transform()
{
}

MeshInstance::MeshInstance(vx::StringID meshSid, vx::StringID materialSid, const vx::Transform &transform)
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

void MeshInstanceFile::load(const u8 *ptr)
{
	memcpy(this, ptr, sizeof(MeshInstanceFile));
}

/*void MeshInstanceFile::load(const YAML::Node &node)
{
	auto strMesh = node["mesh"].as<std::string>();
	auto strMaterial = node["material"].as<std::string>();
	m_transform.m_translation = node["translation"].as<vx::float3>();
	m_transform.m_rotation = node["rotation"].as<vx::float3>();
	m_transform.m_scaling = node["scaling"].as<f32>();

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
}*/

const char* MeshInstanceFile::getMeshFile() const noexcept
{
	return m_mesh;
}

const char* MeshInstanceFile::getMaterialFile() const noexcept
{
	return m_material;
}