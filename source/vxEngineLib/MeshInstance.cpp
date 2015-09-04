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
#include <vxEngineLib/MeshInstance.h>
#include <cstring>
#include <vxEngineLib/Material.h>
#include <vxLib/Graphics/Mesh.h>

MeshInstance::MeshInstance()
	:m_nameSid(),
	m_meshSid(),
	m_material(),
	m_animationSid(),
	m_transform(),
	m_bounds(),
	m_rigidBodyType()
{
}

MeshInstance::MeshInstance(const MeshInstance &rhs)
	:m_nameSid(rhs.m_nameSid),
	m_meshSid(rhs.m_meshSid),
	m_material(rhs.m_material),
	m_animationSid(rhs.m_animationSid),
	m_transform(rhs.m_transform),
	m_bounds(rhs.m_bounds),
	m_rigidBodyType(rhs.m_rigidBodyType)
{

}

MeshInstance::MeshInstance(MeshInstance &&rhs)
	:m_nameSid(std::move(rhs.m_nameSid)),
	m_meshSid(std::move(rhs.m_meshSid)),
	m_material(std::move(rhs.m_material)),
	m_animationSid(std::move(rhs.m_animationSid)),
	m_transform(std::move(rhs.m_transform)),
	m_bounds(rhs.m_bounds),
	m_rigidBodyType(rhs.m_rigidBodyType)
{

}

MeshInstance::MeshInstance(const MeshInstanceDesc &desc)
	:m_nameSid(desc.nameSid),
	m_meshSid(desc.meshSid),
	m_material(desc.material),
	m_animationSid(desc.animationSid),
	m_transform(desc.transform),
	m_bounds(desc.bounds),
	m_rigidBodyType(desc.rigidBodyType)
{
}

MeshInstance::~MeshInstance()
{

}

MeshInstance& MeshInstance::operator = (const MeshInstance &rhs)
{
	if (this != &rhs)
	{
		m_nameSid = rhs.m_nameSid;
		m_meshSid = rhs.m_meshSid;
		m_material = rhs.m_material;
		m_animationSid = rhs.m_animationSid;
		m_transform = rhs.m_transform;
		m_bounds = rhs.m_bounds;
		m_rigidBodyType = rhs.m_rigidBodyType;
	}
	return *this;
}

MeshInstance& MeshInstance::operator = (MeshInstance &&rhs)
{
	if (this != &rhs)
	{
		m_nameSid = std::move(rhs.m_nameSid);
		m_meshSid = std::move(rhs.m_meshSid);
		m_material = std::move(rhs.m_material);
		m_animationSid = std::move(rhs.m_animationSid);
		m_transform = std::move(rhs.m_transform);
		std::swap(m_bounds, rhs.m_bounds);
		m_rigidBodyType = rhs.m_rigidBodyType;
	}
	return *this;
}

void MeshInstance::setTranslation(const vx::float3 &translation)
{
	m_transform.m_translation = translation;

}

const Material* MeshInstance::getMaterial() const noexcept
{
	return m_material;
}

void MeshInstance::setMaterial(const Material* material)
{
	m_material = material;
}

void MeshInstance::setBounds(const vx::Mesh &mesh)
{
	auto vertexCount = mesh.getVertexCount();
	auto vertices = mesh.getVertices();

	AABB bounds;
	for (auto i = 0u; i < vertexCount; ++i)
	{
		bounds.max = vx::max(bounds.max, vertices[i].position);
		bounds.min = vx::min(bounds.min, vertices[i].position);
	}

	m_bounds = bounds;
}