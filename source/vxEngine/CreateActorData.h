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

class Scene;

namespace physx
{
	class PxController;
}

#include <vxLib/StringID.h>
#include <vxEngineLib/Transform.h>

class CreateActorData
{
	physx::PxController* m_controller;
	vx::Transform m_transform;
	vx::StringID m_mesh;
	vx::StringID m_material;
	f32 m_height;
	u16 m_spawnIndex;
	u16 m_gpuIndex;
	u8 m_flags;

public:
	CreateActorData(const vx::Transform &transform, const vx::StringID &meshSid, const vx::StringID &materialSid, f32 height, u16 spawnIndex)
		:m_controller(nullptr),
		m_transform(transform),
		m_mesh(meshSid),
		m_material(materialSid),
		m_height(height),
		m_spawnIndex(spawnIndex),
		m_gpuIndex(0),
		m_flags(0)
	{
	}

	void setPhysx(physx::PxController* controller)
	{
		const auto flag = 1 << 0;

		VX_ASSERT(controller);
		m_controller = controller;

		m_flags |= flag;
	}

	void setGpu(u16 gpuIndex)
	{
		const auto flag = 1 << 1;

		m_gpuIndex = gpuIndex;

		m_flags |= flag;
	}

	f32 getHeight() const { return m_height; }

	bool isValid() const
	{
		const auto flag = 1 << 0 | 1 << 1;

		return ((m_flags & flag) == flag);
	}

	const vx::Transform& getTransform() const
	{
		return m_transform;
	}

	const vx::StringID& getMeshSid() const
	{
		return m_mesh;
	}

	const vx::StringID& getMaterialSid() const
	{
		return m_material;
	}

	physx::PxController* getController() const
	{
		return m_controller;
	}

	u16 getGpuIndex() const
	{
		return m_gpuIndex;
	}
};