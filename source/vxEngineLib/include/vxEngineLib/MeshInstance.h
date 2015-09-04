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

#include <vxEngineLib/PhysxEnums.h>
#include <vxLib/StringID.h>
#include "Transform.h"
#include <vxEngineLib/AABB.h>


struct MeshInstanceDesc
{
	vx::StringID nameSid;
	vx::StringID meshSid;
	const Material* material;
	vx::StringID animationSid;
	vx::Transform transform;
	AABB bounds;
	PhysxRigidBodyType rigidBodyType;
};

class MeshInstance
{
	vx::StringID m_nameSid;
	vx::StringID m_meshSid;
	const Material* m_material;
	vx::StringID m_animationSid;
	vx::Transform m_transform;
	AABB m_bounds;
	PhysxRigidBodyType m_rigidBodyType;

public:
	MeshInstance();
	MeshInstance(const MeshInstance &rhs);
	MeshInstance(MeshInstance &&rhs);
	MeshInstance(const MeshInstanceDesc &desc);
	~MeshInstance();

	MeshInstance& operator=(const MeshInstance &rhs);
	MeshInstance& operator=(MeshInstance &&rhs);

	const vx::StringID& getNameSid() const{ return m_nameSid; }
	const vx::StringID& getMeshSid() const { return m_meshSid; }
	const Material* getMaterial() const noexcept;
	vx::StringID getAnimationSid() const noexcept{ return m_animationSid; }
	const vx::Transform& getTransform() const noexcept { return m_transform; }
	PhysxRigidBodyType getRigidBodyType() const { return m_rigidBodyType; }
	const AABB& getBounds() const { return m_bounds; }

	void setTranslation(const vx::float3 &translation);
	void setRotation(const vx::float4 &qRotation) { m_transform.m_qRotation = qRotation; }
	void setMaterial(const Material* material);
	void setNameSid(const vx::StringID &sid) { m_nameSid = sid; }
	void setMeshSid(const vx::StringID &sid) { m_meshSid = sid; }
	void setAnimationSid(const vx::StringID &sid) { m_animationSid = sid; }
	void setRigidBodyType(PhysxRigidBodyType type) { m_rigidBodyType = type; }
	void setBounds(const AABB &bounds) { m_bounds = bounds; }

	void setBounds(const vx::Mesh &mesh);
};