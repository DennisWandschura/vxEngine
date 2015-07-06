#pragma once
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
#include <vxEngineLib/Reference.h>

class MeshInstance
{
	vx::StringID m_nameSid;
	vx::StringID m_meshSid;
	Reference<Material> m_material;
	vx::StringID m_animationSid;
	vx::Transform m_transform;

public:
	MeshInstance();
	MeshInstance(const MeshInstance &rhs);
	MeshInstance(MeshInstance &&rhs);
	MeshInstance(const vx::StringID &nameSid, const vx::StringID &meshSid, const Reference<Material> &material, const vx::StringID &animationSid, const vx::Transform &transform);
	~MeshInstance();

	MeshInstance& operator=(const MeshInstance &rhs);
	MeshInstance& operator=(MeshInstance &&rhs);

	vx::StringID getNameSid() const noexcept{ return m_nameSid; }
	vx::StringID getMeshSid() const noexcept { return m_meshSid; }
	const Reference<Material>& getMaterial() const noexcept;
	vx::StringID getAnimationSid() const noexcept{ return m_animationSid; }
	const vx::Transform& getTransform() const noexcept { return m_transform; }

	void setTranslation(const vx::float3 &translation);
	void setRotation(const vx::float4 &qRotation) { m_transform.m_qRotation = qRotation; }
	void setMaterial(const Reference<Material> &material);
	void setNameSid(const vx::StringID &sid) { m_nameSid = sid; }
	void setMeshSid(const vx::StringID &sid) { m_meshSid = sid; }
	void setAnimationSid(const vx::StringID &sid) { m_animationSid = sid; }
};