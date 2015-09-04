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

#include <vxEngineLib/MeshInstanceFile.h>

MeshInstanceFileV8::MeshInstanceFileV8()
	:m_name(),
	m_mesh(),
	m_material(),
	m_animation(),
	m_transform(),
	m_bounds(),
	m_rigidBodyType(),
	m_padding()
{
}

MeshInstanceFileV8::MeshInstanceFileV8(const char(&instanceName)[32], const char(&meshName)[32], const char(&materialName)[32], const char(&animationName)[32], const vx::Transform &transform, const AABB &bounds, PhysxRigidBodyType rigidBodyType)
	:m_name(),
	m_mesh(),
	m_material(),
	m_animation(),
	m_transform(transform),
	m_bounds(bounds),
	m_rigidBodyType(rigidBodyType),
	m_padding()
{
	strncpy(m_name, instanceName, 32);
	strncpy(m_mesh, meshName, 32);
	strncpy(m_material, materialName, 32);
	strncpy(m_animation, animationName, 32);
}

void MeshInstanceFileV8::convert(const MeshInstanceFileV4 &rhs)
{
	strcpy_s(m_name, rhs.getName());
	strcpy_s(m_mesh, rhs.getMeshFile());
	strcpy_s(m_material, rhs.getMaterialFile());
	strcpy_s(m_animation, rhs.getAnimation());
	m_transform = rhs.getTransform();
}

void MeshInstanceFileV8::convert(const MeshInstanceFileV5 &rhs)
{
	strncpy(m_name, rhs.m_name, 32);
	strncpy(m_mesh, rhs.m_mesh, 32);
	strncpy(m_material, rhs.m_material, 32);
	strncpy(m_animation, rhs.m_animation, 32);
	m_transform = rhs.m_transform;
	m_rigidBodyType = rhs.m_rigidBodyType;
	memcpy(m_padding, rhs.m_padding, sizeof(m_padding));
}