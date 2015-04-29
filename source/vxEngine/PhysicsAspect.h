#pragma once

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
	class Mesh;

	namespace gl
	{
		class StateManager;
		class ProgramPipeline;
	}
}

class Scene;
class FileAspect;
class MeshInstance;

#include "EventListener.h"
#include <PhysX/foundation/PxErrorCallback.h>
#include <PhysX/extensions/PxDefaultAllocator.h>
#include "LoadFileCallback.h"
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

class PhysicsAspect : public EventListener
{
	static UserErrorCallback s_defaultErrorCallback;
	static physx::PxDefaultAllocator s_defaultAllocatorCallback;

	physx::PxScene* m_pScene;
	physx::PxControllerManager* m_pControllerManager;
	FileAspect &m_fileAspect;
	physx::PxMaterial* m_pActorMaterial{ nullptr };
	physx::PxPhysics *m_pPhysics;
	std::mutex m_mutex;
	std::vector<const physx::PxTriangleMesh*> m_triangleMeshInstances;
	vx::sorted_vector<vx::StringID64, physx::PxTriangleMesh*> m_physxMeshes;
	vx::sorted_vector<vx::StringID64, physx::PxMaterial*> m_physxMaterials;
	vx::sorted_vector<const MeshInstance*, physx::PxRigidStatic*> m_staticMeshInstances;
	physx::PxFoundation *m_pFoundation;
	physx::PxDefaultCpuDispatcher* m_pCpuDispatcher;
	physx::PxCooking* m_pCooking;

	physx::PxTriangleMesh* processMesh(const vx::Mesh* pMesh);
	void processScene(const Scene* pScene);

	//////////////// handle Events
	void handleFileEvent(const Event &evt);
	////////////////

public:
	explicit PhysicsAspect(FileAspect &fileAspect);

	bool initialize();
	void shutdown();

	void fetch();
	void update(const F32 dt);

	void handleEvent(const Event &evt);

	physx::PxController* createActor(const vx::float3 &translation, F32 height);

	void move(const vx::float4a &velocity, F32 dt, physx::PxController* pController);
	void setPosition(const vx::float3 &position, physx::PxController* pController);

	MeshInstance* raycast_static(const vx::float3 &origin, const vx::float3 &unitDir, F32 maxDist, vx::float3* hitPosition) const;

	bool editorGetStaticMeshInstancePosition(const MeshInstance* ptr, vx::float3* p) const;
	void editorSetStaticMeshInstancePosition(const MeshInstance* ptr, const vx::float3 &p);
};