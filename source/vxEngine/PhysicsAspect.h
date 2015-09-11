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
	class PxPhysics;
	class PxScene;
	class PxDefaultCpuDispatcher;
	class PxCooking;
	class PxTriangleMesh;
	class PxMaterial;
	class PxController;
	class PxControllerManager;
	class PxRigidStatic;
	class PxShape;
	class PxRigidDynamic;
	class PxJoint;
	class PxRigidActor;
	struct PxQueryFilterData;
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
class MySimulationCallback;
class MyHitReportCallback;
struct Joint;
class Event;

#include <vxEngineLib/MessageListener.h>
#include <vxLib/Container/sorted_vector.h>
#include <vxLib/StringID.h>
#include <vxLib/math/Vector.h>
#include <foundation/PxErrorCallback.h>
#include <extensions/PxDefaultAllocator.h>
#include <vector>
#include "PhysicsCpuDispatcher.h"
#include <vxEngineLib/Event.h>

enum class PhsyxMeshType : u32;
enum class PhysxRigidBodyType : u8;

class UserErrorCallback : public physx::PxErrorCallback
{
public:
	void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override;
};

struct PhysicsHitData
{
	vx::StringID sid;
	vx::float3 hitPosition;
	f32 distance;
	physx::PxRigidActor* actor;
};

class PhysicsAspect : public vx::MessageListener
{
	static UserErrorCallback s_defaultErrorCallback;
	static physx::PxDefaultAllocator s_defaultAllocatorCallback;

protected:
	physx::PxScene* m_pScene;
	physx::PxControllerManager* m_pControllerManager;
	physx::PxMaterial* m_pActorMaterial{ nullptr };
	physx::PxPhysics *m_pPhysics;
	physx::PxRigidDynamic* m_humanActor;
	Event m_evtFetch;
	std::atomic_uint m_currentWriterTid;
	std::atomic_int m_writerCount;
	std::atomic_int m_flag;
	std::vector<Event> m_blockEvents;
	vx::sorted_vector<vx::StringID, PhsyxMeshType> m_physxMeshTypes;
	vx::sorted_vector<vx::StringID, physx::PxConvexMesh*> m_physxConvexMeshes;
	vx::sorted_vector<vx::StringID, physx::PxTriangleMesh*> m_physxMeshes;
	vx::sorted_vector<vx::StringID, physx::PxMaterial*> m_physxMaterials;
	vx::sorted_vector<vx::StringID, physx::PxRigidStatic*> m_staticMeshInstances;
	vx::sorted_vector<vx::StringID, physx::PxRigidDynamic*> m_dynamicMeshInstances;
	vx::sorted_vector<vx::StringID, PhysxRigidBodyType> m_meshInstances;
	std::vector<physx::PxJoint*> m_joints;
	physx::PxFoundation *m_pFoundation;
	PhysicsCpuDispatcher m_cpuDispatcher;
	physx::PxDefaultCpuDispatcher* m_pCpuDispatcher;
	physx::PxCooking* m_pCooking;
	MyHitReportCallback* m_callback;
	MySimulationCallback* m_mySimCallback;

	physx::PxTriangleMesh* processTriangleMesh(const vx::MeshFile &mesh);
	physx::PxConvexMesh* processMeshConvex(const vx::MeshFile &mesh);
	bool addMesh(const vx::StringID &sid, const vx::MeshFile &meshFile);
	void processScene(const Scene* pScene);

	physx::PxConvexMesh* getConvexMesh(const vx::StringID &sid, const vx::MeshFile &meshFile);
	physx::PxTriangleMesh* getTriangleMesh(const vx::StringID &sid, const vx::MeshFile &meshFile);

	//////////////// handle Events
	void handleFileEvent(const vx::Message &evt);
	void handleIngameMessage(const vx::Message &evt);
	////////////////

	void addDynamicMeshInstance(const physx::PxTransform &transform, physx::PxShape &shape, const vx::StringID &instanceSid, void** outData);
	void addStaticMeshInstance(const physx::PxTransform &transform, physx::PxShape &shape, const vx::StringID &instanceSid);
	void addMeshInstanceImpl(const MeshInstance &instance, void** outData);

	bool raycast(const physx::PxVec3 &origin, const physx::PxVec3 &unitDir, const physx::PxQueryFilterData &filterData, f32 maxDistance, PhysicsHitData* hitData) const;
	bool raycastNoPlayer(const physx::PxVec3 &origin, const physx::PxVec3 &unitDir, const physx::PxQueryFilterData &filterData, f32 maxDistance, PhysicsHitData* hitData) const;

	physx::PxRigidActor* PhysicsAspect::getRigidActor(const vx::StringID &sid, PhysxRigidBodyType* outType);

	void lockSceneWrite();
	void unlockSceneWrite();

public:
	PhysicsAspect();
	virtual ~PhysicsAspect();

	bool initialize(vx::TaskManager* taskManager);
	void shutdown();

	void fetch();
	void update(const f32 dt);

	const Event& getEventPhysicsFetch() const {return m_evtFetch;}
	void addBlockEvent(const Event &evt);

	virtual void handleMessage(const vx::Message &evt) override;

	physx::PxController* createActor(const vx::StringID &mesh, const vx::float3 &translation);
	physx::PxController* createActor(const vx::float3 &translation, f32 height);
	physx::PxJoint* createJoint(const Joint &joint);
	physx::PxJoint* createSphericalJoint(physx::PxRigidActor* actor);
	physx::PxJoint* createFixedJoint(physx::PxRigidActor* actor);
	physx::PxJoint* createD6Joint(physx::PxRigidActor* actor);

	void move(const vx::float4a &velocity, f32 dt, physx::PxController* pController);
	
	bool raycast_staticDynamic(const vx::float3 &origin, const vx::float3 &unitDir, f32 maxDist, PhysicsHitData* hitData) const;
	bool raycast_staticDynamic(const vx::float3 &origin, const vx::float4a &unitDir, f32 maxDist, PhysicsHitData* hitData) const;
	bool raycast_staticDynamic(const vx::float4a &origin, const vx::float4a &unitDir, f32 maxDist, PhysicsHitData* hitData) const;

	bool raycastDynamic(const vx::float3 &origin, const vx::float3 &unitDir, f32 maxDist, PhysicsHitData* hitData) const;

	physx::PxRigidStatic* getStaticMesh(const vx::StringID &sid);
	physx::PxRigidDynamic* getDynamicMesh(const vx::StringID &sid);

	void setHumanActor(physx::PxRigidDynamic* actor);
};