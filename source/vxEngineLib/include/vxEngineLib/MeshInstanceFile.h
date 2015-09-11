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

	const char* getName() const noexcept { return m_name; }
	const char* getMeshFile() const noexcept { return m_mesh; }
	const char* getMaterialFile() const noexcept { return m_material; }
	const char* getAnimation() const noexcept { return m_animation; }
	const vx::Transform& getTransform() const noexcept { return m_transform; }
	PhysxRigidBodyType getRigidBodyType() const { return m_rigidBodyType; }
	const AABB& getBounds() const { return m_bounds; }
};