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
#pragma once

class Material;

namespace vx
{
	class Mesh;
}

namespace YAML
{
	class Node;
}

#include <vxLib/StringID.h>
#include "Transform.h"

class MeshInstance
{
	vx::StringID m_nameSid;
	vx::StringID m_meshSid;
	vx::StringID m_materialSid;
	vx::Transform m_transform;

public:
	MeshInstance();
	MeshInstance(const vx::StringID &nameSid, const vx::StringID &meshSid, const vx::StringID &materialSid, const vx::Transform &transform);

	vx::StringID getNameSid() const noexcept{ return m_nameSid; }
	vx::StringID getMeshSid() const noexcept { return m_meshSid; }
	vx::StringID getMaterialSid() const noexcept { return m_materialSid; }
	const vx::Transform& getTransform() const noexcept { return m_transform; }

#if _VX_EDITOR
	void setTranslation(const vx::float3 &translation);
#endif
};

class MeshInstanceFile
{
	using Buffer = char[32];

	Buffer m_name;
	Buffer m_mesh;
	Buffer m_material;
	vx::Transform m_transform;

public:
	MeshInstanceFile();
	MeshInstanceFile(const char(&instanceName)[32], const char(&meshName)[32], const char(&materialName)[32], const vx::Transform &transform);

	const char* getName() const noexcept{ return m_name; }
	const char* getMeshFile() const noexcept;
	const char* getMaterialFile() const noexcept;
	const vx::Transform& getTransform() const noexcept{ return m_transform; }
};