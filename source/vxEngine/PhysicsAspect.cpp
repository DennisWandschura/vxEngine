#include "PhysicsAspect.h"
#include <PhysX/PxPhysicsAPI.h>
#include <vxLib/Graphics/Mesh.h>
#include "Scene.h"
#include "MeshInstance.h"
#include <Windows.h>
#include "Material.h"
#include <PhysX/extensions/PxDefaultSimulationFilterShader.h>
#include "FileAspect.h"
#include "PhysicsDefines.h"
#include "enums.h"
#include "Event.h"
#include "EventTypes.h"

UserErrorCallback PhysicsAspect::s_defaultErrorCallback{};
physx::PxDefaultAllocator PhysicsAspect::s_defaultAllocatorCallback{};

void UserErrorCallback::reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
{
	// error processing implementation
	//...

	printf("Physx Error: %d, %s %s %d\n", code, message, file, line);
}

PhysicsAspect::PhysicsAspect(FileAspect &fileAspect)
	:m_fileAspect(fileAspect)
{
}

bool PhysicsAspect::initialize()
{
	m_pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, s_defaultAllocatorCallback,
		s_defaultErrorCallback);

	if (!m_pFoundation)
	{
		return false;
		//	fatalError("PxCreateFoundation failed!");
	}

	bool recordMemoryAllocations = true;
	/*	m_pProfileZoneManager = &physx::PxProfileZoneManager::createProfileZoneManager(m_pFoundation);
		if (!m_pProfileZoneManager)
		{
		//fatalError("PxProfileZoneManager::createProfileZoneManager failed!");
		return false;
		}*/

	physx::PxTolerancesScale toleranceScale;
	m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pFoundation,
		toleranceScale, recordMemoryAllocations);
	if (!m_pPhysics)
	{
		//VX_ASSERT(false, "PxCreatePhysics failed!");
		return false;
	}

	m_pCooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_pFoundation, physx::PxCookingParams(toleranceScale));
	if (!m_pCooking)
	{
		//fatalError("PxCreateCooking failed!");
		return false;
	}

	m_pCpuDispatcher = physx::PxDefaultCpuDispatcherCreate(2);

	physx::PxSceneDesc sceneDesc(toleranceScale);
	sceneDesc.cpuDispatcher = m_pCpuDispatcher;
	sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);

	if (!sceneDesc.filterShader)
		sceneDesc.filterShader = &physx::PxDefaultSimulationFilterShader;

	m_pScene = m_pPhysics->createScene(sceneDesc);
	if (!m_pScene)
	{
		return false;
	}

	m_pActorMaterial = m_pPhysics->createMaterial(1, 1, 0.1f);

	m_pControllerManager = PxCreateControllerManager(*m_pScene);

	return true;
}

void PhysicsAspect::shutdown()
{
	m_pControllerManager->release();
	m_pCooking->release();
	m_pPhysics->release();
	m_pFoundation->release();
}

void PhysicsAspect::fetch()
{
	m_pScene->fetchResults(true);
}

void PhysicsAspect::update(const F32 dt)
{
	m_pScene->simulate(dt);
}

void PhysicsAspect::handleEvent(const Event &evt)
{
	switch (evt.type)
	{
	case(EventType::File_Event) :
		handleFileEvent(evt);
		break;
	default:
		break;
	}
}

void PhysicsAspect::handleFileEvent(const Event &evt)
{
	auto fileEvent = (FileEvent)evt.code;
	switch (fileEvent)
	{
	case FileEvent::Scene_Loaded:
		processScene((Scene*)evt.arg1.ptr);
		break;
	default:
		break;
	}
}

U8 PhysicsAspect::raycast_static(const vx::float3 &o, const vx::float3 &dir, F32 maxDistance, vx::float3* hitPosition) const
{
	physx::PxVec3 origin(o.x, o.y,o.z);                 // [in] Ray origin
	physx::PxVec3 unitDir(dir.x, dir.y, dir.z);                // [in] Normalized ray direction
	physx::PxRaycastBuffer hit;                 // [out] Raycast results

	// [in] Define filter for static objects only
	physx::PxQueryFilterData filterData(physx::PxQueryFlag::eSTATIC);

	//unitDir.normalize();

	m_pScene->raycast((physx::PxVec3)origin, unitDir, maxDistance, hit, physx::PxHitFlag::eDEFAULT, filterData);

	U8 result = hit.hasBlock;
	if (result != 0)
	{
		hitPosition->x = hit.block.position.x;
		hitPosition->y = hit.block.position.y;
		hitPosition->z = hit.block.position.z;
	}

	return result;
}

void PhysicsAspect::processScene(const Scene* pScene)
{
	m_pScene->lockWrite();

	//auto playerSpawn = pScene->getPlayerSpawn();

	auto pPlaneMaterial = m_pPhysics->createMaterial(1, 1, 0);
	physx::PxRigidStatic* plane = physx::PxCreatePlane(*m_pPhysics, physx::PxPlane(physx::PxVec3(0, 1, 0), 0), *pPlaneMaterial);

	auto &meshes = pScene->getMeshes();

	for (auto i = 0u; i < meshes.size(); ++i)
	{
		auto pMesh = meshes[i];
		auto pResult = processMesh(pMesh);
		if (!pResult)
		{
			puts("error processing mesh");
		}

		// need to lock because we are modifying the vector
		std::lock_guard<std::mutex> guard(m_mutex);
		m_physxMeshes.insert(meshes.keys()[i], pResult);
	}

	auto numInstances = pScene->getMeshInstanceCount();
	auto pMeshInstances = pScene->getMeshInstances();

	auto &sceneMaterials = pScene->getMaterials();
	for (auto i = 0u; i < sceneMaterials.size(); ++i)
	{
		auto pCurrentMaterial = sceneMaterials[i];
		auto sid = sceneMaterials.keys()[i];
		auto pMaterial = m_pPhysics->createMaterial(pCurrentMaterial->getStaticFriction(), pCurrentMaterial->getDynamicFriction(), pCurrentMaterial->getRestitution());
		assert(pMaterial);
		m_physxMaterials.insert(sid, pMaterial);
	}

	for (auto i = 0u; i < numInstances; ++i)
	{
		auto &meshInstance = pMeshInstances[i];
		auto meshSid = meshInstance.getMeshSid();
		auto instanceTransform = meshInstance.getTransform();

		auto qRotation = vx::loadFloat(instanceTransform.m_rotation);
		qRotation = vx::QuaternionRotationRollPitchYawFromVector(qRotation);

		physx::PxTransform transform;
		transform.p.x = instanceTransform.m_translation.x;
		transform.p.y = instanceTransform.m_translation.y;
		transform.p.z = instanceTransform.m_translation.z;
		transform.q.x = qRotation.m128_f32[0];
		transform.q.y = qRotation.m128_f32[1];
		transform.q.z = qRotation.m128_f32[2];
		transform.q.w = qRotation.m128_f32[3];

		assert(transform.isValid());

		auto itPhysxTriangleMesh = m_physxMeshes.find(meshSid);
		assert(itPhysxTriangleMesh != m_physxMeshes.end());

		auto itPhysxMaterial = m_physxMaterials.find(meshInstance.getMaterialSid());

		//auto isActor = meshInstance.isActor();

		auto pmat = *itPhysxMaterial;

		// static
		auto pShape = m_pPhysics->createShape(physx::PxTriangleMeshGeometry(*itPhysxTriangleMesh), *pmat);
		auto pRigidStatic = m_pPhysics->createRigidStatic(transform);
		pRigidStatic->attachShape(*pShape);

		m_pScene->addActor(*pRigidStatic);

		m_triangleMeshInstances.push_back(*itPhysxTriangleMesh);
	}

	m_pScene->unlockWrite();
}

physx::PxTriangleMesh* PhysicsAspect::processMesh(const vx::Mesh* pMesh)
{
	auto pMeshVertices = pMesh->getVertices();
	auto pIndices = pMesh->getIndices();

	auto vertexCount = pMesh->getVertexCount();
	auto indexCount = pMesh->getIndexCount();

	auto pVertices = std::make_unique<vx::float3[]>(vertexCount);
	for (U32 i = 0; i < vertexCount; ++i)
	{
		pVertices[i] = pMeshVertices[i].position;
	}

	physx::PxTriangleMeshDesc meshDesc;
	meshDesc.points.count = vertexCount;
	meshDesc.points.stride = sizeof(vx::float3);
	meshDesc.points.data = pVertices.get();

	meshDesc.triangles.count = indexCount / 3;
	meshDesc.triangles.stride = 3 * sizeof(U32);
	meshDesc.triangles.data = pIndices;

	physx::PxDefaultMemoryOutputStream writeBuffer;
	bool status = m_pCooking->cookTriangleMesh(meshDesc, writeBuffer);
	if (!status)
	{
		return nullptr;
	}

	physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());

	return m_pPhysics->createTriangleMesh(readBuffer);
}

physx::PxController* PhysicsAspect::createActor(const vx::float3 &translation, F32 height)
{
	physx::PxCapsuleControllerDesc desc;
	desc.height = height;
	desc.radius = 0.2f;
	desc.position.x = translation.x;
	desc.position.y = translation.y;
	desc.position.z = translation.z;
	desc.stepOffset = 0.1f;

	desc.material = m_pActorMaterial;

	assert(desc.isValid());

	m_pScene->lockWrite();

	auto p = m_pControllerManager->createController(desc);

	m_pScene->unlockWrite();

	assert(p);

	return p;
}

void PhysicsAspect::move(const vx::float4a &velocity, F32 dt, physx::PxController* pController)
{
	physx::PxControllerFilters filters;
	//filters.

	physx::PxVec3 v;
	v.x = velocity.x;
	v.y = velocity.y;
	v.z = velocity.z;

	pController->move(v, 0.0001f, dt, filters);
}

void PhysicsAspect::setPosition(const vx::float3 &position, physx::PxController* pController)
{
	pController->setPosition(physx::PxExtendedVec3(position.x, position.y, position.z));
}