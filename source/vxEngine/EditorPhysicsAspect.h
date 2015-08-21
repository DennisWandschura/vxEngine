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

class ArrayAllocator;

namespace physx
{
	class PxDefaultMemoryOutputStream;
}

namespace vx
{
	class Mesh;
}

template<typename T>
class Reference;

#include "PhysicsAspect.h"

namespace Editor
{
	class Scene;

	class PhysicsAspect : public ::PhysicsAspect
	{
		void handleFileEvent(const vx::Message &evt);

		void processScene(const Scene* pScene);

		bool createTriangleMesh(const vx::Mesh &mesh, physx::PxDefaultMemoryOutputStream* writeBuffer);
		bool createConvexMesh(const vx::Mesh &mesh, physx::PxDefaultMemoryOutputStream* writeBuffer);

	public:
		PhysicsAspect();
		~PhysicsAspect();

		void handleMessage(const vx::Message &evt) override;

		void setPosition(const vx::float3 &position, physx::PxController* pController);

		bool editorGetStaticMeshInstancePosition(const vx::StringID &sid, vx::float3* p) const;
		void editorSetStaticMeshInstanceTransform(const ::MeshInstance &meshInstance, const vx::StringID &sid);
		void editorSetStaticMeshInstanceMesh(const ::MeshInstance &instance);

		physx::PxCooking* getCooking() { return m_pCooking; }

		bool setMeshPhysxType(vx::MeshFile* meshFile, PhsyxMeshType type, ArrayAllocator* meshDataAllocator);
		bool setMeshInstanceRigidBodyType(const vx::StringID &instanceSid, const ::MeshInstance &meshInstance, PhysxRigidBodyType rigidBodyType);

		void addMeshInstance(const ::MeshInstance &instance);

		bool createJoint(const Joint &joint);

		void saveScene();
	};
}