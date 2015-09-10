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

#include <vxEngineLib/PhysxEnums.h>
#include "Transform.h"
#include "AABB.h"

class MeshInstanceFileV8;

class MeshInstanceFileV4
{
	using Buffer = char[32];

	Buffer m_name;
	Buffer m_mesh;
	Buffer m_material;
	Buffer m_animation;
	vx::Transform m_transform;

public:
	MeshInstanceFileV4();
	MeshInstanceFileV4(const char(&instanceName)[32], const char(&meshName)[32], const char(&materialName)[32], const char(&animationName)[32], const vx::Transform &transform);

	void convert(const MeshInstanceFileV8 &rhs);

	const char* getName() const noexcept{ return m_name; }
	const char* getMeshFile() const noexcept{ return m_mesh; }
	const char* getMaterialFile() const noexcept{ return m_material; }
	const char* getAnimation() const noexcept{ return m_animation; }
	const vx::Transform& getTransform() const noexcept{ return m_transform; }
};

class MeshInstanceFileV5
{
	friend MeshInstanceFileV8;

	using Buffer = char[32];

	Buffer m_name;
	Buffer m_mesh;
	Buffer m_material;
	Buffer m_animation;
	vx::Transform m_transform;
	PhysxRigidBodyType m_rigidBodyType;
	u8 m_padding[3];

public:
	MeshInstanceFileV5();
	MeshInstanceFileV5(const MeshInstanceFileV5 &);
	MeshInstanceFileV5(MeshInstanceFileV5 &&);
	MeshInstanceFileV5(const char(&instanceName)[32], const char(&meshName)[32], const char(&materialName)[32], const char(&animationName)[32], const vx::Transform &transform, PhysxRigidBodyType rigidBodyType);
	~MeshInstanceFileV5();

	MeshInstanceFileV5& operator=(const MeshInstanceFileV5 &rhs);

	void convert(const MeshInstanceFileV4 &rhs);
	void convert(const MeshInstanceFileV8 &rhs);

	const char* getName() const noexcept{ return m_name; }
	const char* getMeshFile() const noexcept{ return m_mesh; }
	const char* getMaterialFile() const noexcept{ return m_material; }
	const char* getAnimation() const noexcept{ return m_animation; }
	const vx::Transform& getTransform() const noexcept{ return m_transform; }
	PhysxRigidBodyType getRigidBodyType() const { return m_rigidBodyType; }
};

struct MeshInstanceFileV8Desc
{
	char instanceName[32];
	char meshName[32];
	char materialName[32];
	char animationName[32];
	vx::Transform transform;
	AABB bounds; 
	PhysxRigidBodyType rigidBodyType;
};

class MeshInstanceFileV8
{
	friend MeshInstanceFileV5;

	using Buffer = char[32];

	Buffer m_name;
	Buffer m_mesh;
	Buffer m_material;
	Buffer m_animation;
	vx::Transform m_transform;
	AABB m_bounds;
	PhysxRigidBodyType m_rigidBodyType;
	u8 m_padding[3];

public:
	MeshInstanceFileV8();
	explicit MeshInstanceFileV8(const MeshInstanceFileV8Desc &desc);

	void convert(const MeshInstanceFileV4 &rhs);
	void convert(const MeshInstanceFileV5 &rhs);

	const char* getName() const noexcept { return m_name; }
	const char* getMeshFile() const noexcept { return m_mesh; }
	const char* getMaterialFile() const noexcept { return m_material; }
	const char* getAnimation() const noexcept { return m_animation; }
	const vx::Transform& getTransform() const noexcept { return m_transform; }
	PhysxRigidBodyType getRigidBodyType() const { return m_rigidBodyType; }
	const AABB& getBounds() const { return m_bounds; }
};