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
	class PxFoundation;
	class PxProfileZoneManager;
	class PxPhysics;
	class PxScene;
	class PxDefaultCpuDispatcher;
	class PxCooking;
	class PxTriangleMesh;
	class PxMaterial;
	class PxController;
	class PxControllerManager;
	class PxRigidStatic;
}

namespace vx
{
	class MeshFile;

	namespace gl
	{
		class StateManager;
		class ProgramPipeline;
	}
}

class Scene;
class FileAspect;
class MeshInstance;

#include <vxEngineLib/EventListener.h>
#include <foundation/PxErrorCallback.h>
#include <extensions/PxDefaultAllocator.h>
#include <mutex>
#include <vxLib/Container/sorted_vector.h>
#include <vxLib/StringID.h>
#include <vxLib/math/Vector.h>
#include <atomic>
#include <vector>

class UserErrorCallback : public physx::PxErrorCallback
{
public:
	void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override;
};

class PhysicsAspect : public vx::EventListener
{
	static UserErrorCallback s_defaultErrorCallback;
	static physx::PxDefaultAllocator s_defaultAllocatorCallback;

protected:
	physx::PxScene* m_pScene;
	physx::PxControllerManager* m_pControllerManager;
	physx::PxMaterial* m_pActorMaterial{ nullptr };
	physx::PxPhysics *m_pPhysics;
	std::mutex m_mutex;
	vx::sorted_vector<vx::StringID, physx::PxConvexMesh*> m_physxConvexMeshes;
	vx::sorted_vector<vx::StringID, physx::PxTriangleMesh*> m_physxMeshes;
	vx::sorted_vector<vx::StringID, physx::PxMaterial*> m_physxMaterials;
	vx::sorted_vector<vx::StringID, physx::PxRigidStatic*> m_staticMeshInstances;
	physx::PxFoundation *m_pFoundation;
	physx::PxDefaultCpuDispatcher* m_pCpuDispatcher;
	physx::PxCooking* m_pCooking;

	physx::PxTriangleMesh* processTriangleMesh(const vx::MeshFile* pMesh);
	physx::PxConvexMesh* processMeshConvex(const vx::MeshFile* pMesh);
	bool processMesh(const vx::StringID &sid, const vx::MeshFile* pMesh, bool* isTriangleMesh);
	void processScene(const Scene* pScene);

	//////////////// handle Events
	void handleFileEvent(const vx::Event &evt);
	void handleIngameEvent(const vx::Event &evt);
	////////////////

	void addMeshInstance(const MeshInstance &instance);

public:
	PhysicsAspect();
	virtual ~PhysicsAspect();

	bool initialize();
	void shutdown();

	void fetch();
	void update(const f32 dt);

	virtual void handleEvent(const vx::Event &evt) override;

	physx::PxController* createActor(const vx::StringID &mesh, const vx::float3 &translation);
	physx::PxController* createActor(const vx::float3 &translation, f32 height);

	void move(const vx::float4a &velocity, f32 dt, physx::PxController* pController);
	
	vx::StringID raycast_static(const vx::float4a &origin, const vx::float4a &unitDir, f32 maxDist, vx::float3* hitPosition) const;
};