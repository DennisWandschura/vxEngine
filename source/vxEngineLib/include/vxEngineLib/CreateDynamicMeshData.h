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

namespace physx
{
	class PxRigidDynamic;
}

#include <vxEngineLib/Transform.h>

class CreateDynamicMeshData
{
	const MeshInstance* m_meshInstance;
	physx::PxRigidDynamic* m_rigidDynamic;
	vx::StringID m_materialSid;
	u32 m_gpuIndex;
	u32 m_count;

	void increment()
	{
		++m_count;
	}

public:
	CreateDynamicMeshData()
		:m_meshInstance(nullptr),
		m_rigidDynamic(nullptr),
		m_materialSid(),
		m_gpuIndex(0xffffffff),
		m_count(0)
	{

	}

	void initialize(const MeshInstance* meshInstance, const vx::StringID &materialSid)
	{
		m_meshInstance = meshInstance;
		m_materialSid = materialSid;
	}

	u32 decrement()
	{
		--m_count;

		return m_count;
	}

	bool isValid() const
	{
		return (m_gpuIndex != 0xffffffff) &&
			(m_rigidDynamic != nullptr);
	}

	void setGpuIndex(u32 gpuIndex)
	{
		m_gpuIndex = gpuIndex;
		increment();
	}

	void setRigidDynamic(physx::PxRigidDynamic* rigidDynamic)
	{
		if (m_rigidDynamic == nullptr)
		{
			m_rigidDynamic = rigidDynamic;
			increment();
		}
	}

	const MeshInstance* getMeshInstance() const
	{
		return m_meshInstance;
	}

	physx::PxRigidDynamic* getRigidDynamic() const
	{
		return m_rigidDynamic;
	}

	u32 getGpuIndex() const
	{
		return m_gpuIndex;
	}

	const vx::StringID& getMaterialSid() const
	{
		return m_materialSid;
	}
};