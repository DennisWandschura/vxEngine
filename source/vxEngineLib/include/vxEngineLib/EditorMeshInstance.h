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

#include "MeshInstance.h"
#include <string>

namespace Editor
{
	class MeshInstance
	{
		::MeshInstance m_meshInstance;
		std::string m_name;

	public:
		MeshInstance(const ::MeshInstance &instance, std::string &&name)
			:m_meshInstance(instance),
			m_name(std::move(name))
		{
		}

		const ::MeshInstance& getMeshInstance() const { return m_meshInstance; }
		::MeshInstance& getMeshInstance() { return m_meshInstance; }
		const std::string& getName() const { return m_name; }

		void setTranslation(const vx::float3 &translation){ m_meshInstance.setTranslation(translation); }
		void setRotation(const vx::float4 &rotation){ m_meshInstance.setRotation(rotation); }
		void setMaterial(const Reference<Material> &material) { m_meshInstance.setMaterial(material); }
		void setMeshSid(const vx::StringID &sid) { m_meshInstance.setMeshSid(sid); }
		void setAnimationSid(const vx::StringID &sid){ m_meshInstance.setAnimationSid(sid); }
		void setRigidBodyType(PhysxRigidBodyType type) { m_meshInstance.setRigidBodyType(type); }

		vx::StringID getNameSid() const noexcept{ return m_meshInstance.getNameSid(); }
		vx::StringID getMeshSid() const noexcept{ return m_meshInstance.getMeshSid(); }
		const Reference<Material>& getMaterial() const noexcept{ return m_meshInstance.getMaterial(); }
		vx::StringID getAnimationSid() const noexcept{ return m_meshInstance.getAnimationSid(); }
		const vx::Transform& getTransform() const noexcept{ return m_meshInstance.getTransform(); }
		PhysxRigidBodyType getRigidBodyType() const { return m_meshInstance.getRigidBodyType(); }
	};
}