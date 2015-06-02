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
	:m_nameSid(),
	m_meshSid(),
	m_materialSid(),
	m_transform()
{
}

MeshInstance::MeshInstance(const vx::StringID &nameSid, const vx::StringID &meshSid, const vx::StringID &materialSid, const vx::Transform &transform)
	:m_nameSid(nameSid),
	m_meshSid(meshSid),
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
	:m_name(),
	m_mesh(),
	m_material(),
	m_transform()
{
}

MeshInstanceFile::MeshInstanceFile(const char(&instanceName)[32], const char(&meshName)[32], const char(&materialName)[32],
	const vx::Transform &transform)
	:m_name(),
	m_mesh(),
	m_material(),
	m_transform(transform)
{
	strcpy_s(m_name, instanceName);
	strcpy_s(m_mesh, meshName);
	strcpy_s(m_material, materialName);
}

const char* MeshInstanceFile::getMeshFile() const noexcept
{
	return m_mesh;
}

const char* MeshInstanceFile::getMaterialFile() const noexcept
{
	return m_material;
}