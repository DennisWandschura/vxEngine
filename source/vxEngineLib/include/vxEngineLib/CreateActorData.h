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
enum class PlayerType : u32;

namespace physx
{
	class PxController;
}

#include <vxLib/StringID.h>
#include <vxEngineLib/Transform.h>
#include <vxEngineLib/Actor.h>
#include <vxEngineLib/smallobject.h>

struct CreateActorDataDesc
{
	vx::Transform transform; 
	vx::StringID actorSid;
	vx::StringID meshSid;
	vx::StringID materialSid;
	f32 height;
	u16 spawnIndex;
	PlayerType type;
	f32 m_fovRad;
	f32 m_maxViewDistance;
};

class CreateActorData : public SmallObject
{
	physx::PxController* m_controller;
	vx::Transform m_transform;
	vx::StringID m_actorSid;
	vx::StringID m_mesh;
	vx::StringID m_material;
	f32 m_height;
	u16 m_spawnIndex;
	u16 m_gpuIndex;
	u8 m_refCount;
	PlayerType m_type;
	f32 m_fovRad;
	f32 m_maxViewDistance;

public:
	explicit CreateActorData(const CreateActorDataDesc &desc)
		:m_controller(nullptr),
		m_transform(desc.transform),
		m_actorSid(desc.actorSid),
		m_mesh(desc.meshSid),
		m_material(desc.materialSid),
		m_height(desc.height),
		m_spawnIndex(desc.spawnIndex),
		m_gpuIndex(0),
		m_refCount(0),
		m_type(desc.type),
		m_fovRad(desc.m_fovRad),
		m_maxViewDistance(desc.m_maxViewDistance)
	{
	}

	void setPhysx(physx::PxController* controller)
	{
		const auto flag = 1 << 0;

		VX_ASSERT(controller);
		m_controller = controller;

		//m_flags |= flag;
		++m_refCount;
	}

	void setGpu(u16 gpuIndex)
	{
		const auto flag = 1 << 1;

		m_gpuIndex = gpuIndex;

		//m_flags |= flag;
		++m_refCount;
	}

	f32 getHeight() const { return m_height; }

	bool isValid() const
	{
		if (m_type == PlayerType::AI)
		{
			return (m_refCount == 2);
		}
		else
		{
			return (m_refCount == 1);
		}
	}

	u32 getRefCount() const 
	{
		return m_refCount;
	}

	void decrement()
	{
		--m_refCount;
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

	const vx::StringID& getActorSid() const
	{
		return m_actorSid;
	}

	physx::PxController* getController() const
	{
		return m_controller;
	}

	u16 getGpuIndex() const
	{
		return m_gpuIndex;
	}

	PlayerType getPlayerType() const
	{
		return m_type;
	}

	f32 getFovRad() const { return m_fovRad; }

	f32 getMaxViewDistance() const { return m_maxViewDistance; }
};