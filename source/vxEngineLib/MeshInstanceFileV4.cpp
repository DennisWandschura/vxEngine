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

MeshInstanceFileV4::MeshInstanceFileV4()
	:m_name(),
	m_mesh(),
	m_material(),
	m_animation(),
	m_transform()
{
}

MeshInstanceFileV4::MeshInstanceFileV4(const char(&instanceName)[32], const char(&meshName)[32], const char(&materialName)[32], const char(&animationName)[32], const vx::Transform &transform)
	:m_name(),
	m_mesh(),
	m_material(),
	m_animation(),
	m_transform(transform)
{
	strcpy_s(m_name, instanceName);
	strcpy_s(m_mesh, meshName);
	strcpy_s(m_material, materialName);
	strcpy_s(m_animation, animationName);
}

void MeshInstanceFileV4::convert(const MeshInstanceFileV8 &rhs)
{
	strcpy_s(m_name, rhs.getName());
	strcpy_s(m_mesh, rhs.getMeshFile());
	strcpy_s(m_material, rhs.getMaterialFile());
	strcpy_s(m_animation, rhs.getAnimation());
	m_transform = rhs.getTransform();
}