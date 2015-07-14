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
#include "PhysicsAspect.h"
#include <PxPhysicsAPI.h>
#include <vxEngineLib/Event.h>
#include <vxEngineLib/EventTypes.h>
#include <vxEngineLib/MeshInstance.h>
#include <vxEngineLib/Locator.h>
#include <vxEngineLib/Material.h>
#include <vxResourceAspect/FileAspect.h>
#include <vxEngineLib/Scene.h>
#include <vxEngineLib/MeshFile.h>
#include <vxEngineLib/FileEvents.h>
#include <vxEngineLib/EventsIngame.h>
#include <vxEngineLib/CreateActorData.h>
#include <vxEngineLib/EventManager.h>

UserErrorCallback PhysicsAspect::s_defaultErrorCallback{};
physx::PxDefaultAllocator PhysicsAspect::s_defaultAllocatorCallback{};

void UserErrorCallback::reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
{
	// error processing implementation
	//...

	printf("Physx Error: %d, %s %s %d\n", code, message, file, line);
}

PhysicsAspect::PhysicsAspect()
	:m_pScene(nullptr),
	m_pControllerManager(nullptr),
	m_pActorMaterial(nullptr),
	m_pPhysics(nullptr),
	m_physxMeshes(),
	m_physxMaterials(),
	m_staticMeshInstances(),
	m_pFoundation(nullptr),
	m_pCpuDispatcher(nullptr),
	m_pCooking(nullptr)
{
}

PhysicsAspect::~PhysicsAspect()
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

	m_pCpuDispatcher = physx::PxDefaultCpuDispatcherCreate(0);

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
	if (m_pControllerManager)
	{
		m_pControllerManager->release();
		m_pControllerManager = nullptr;
	}

	if (m_pCooking)
	{
		m_pCooking->release();
		m_pCooking = nullptr;
	}

	if (m_pPhysics)
	{
		m_pPhysics->release();
		m_pPhysics = nullptr;
	}

	if (m_pFoundation)
	{
		m_pFoundation->release();
		m_pFoundation = nullptr;
	}
}

void PhysicsAspect::fetch()
{
	m_pScene->fetchResults(true);
}

void PhysicsAspect::update(const f32 dt)
{
	m_pScene->simulate(dt);
}

void PhysicsAspect::handleEvent(const vx::Event &evt)
{
	switch (evt.type)
	{
	case(vx::EventType::Ingame_Event) :
		handleIngameEvent(evt);
		break;
	case(vx::EventType::File_Event) :
		handleFileEvent(evt);
		break;
	default:
		break;
	}
}

void PhysicsAspect::handleFileEvent(const vx::Event &evt)
{
	auto fileEvent = (vx::FileEvent)evt.code;
	switch (fileEvent)
	{
	case vx::FileEvent::Scene_Loaded:
		processScene((Scene*)evt.arg2.ptr);
		break;
	default:
		break;
	}
}

void PhysicsAspect::handleIngameEvent(const vx::Event &evt)
{
	auto ingameEvent = (IngameEvent)evt.code;

	if (ingameEvent == IngameEvent::Physx_AddActor)
	{
		CreateActorData* data = (CreateActorData*)evt.arg1.ptr;

		auto transform = data->getTransform();
		auto controller = createActor(transform.m_translation, data->getHeight());
		data->setPhysx(controller);

		vx::Event evt;
		evt.type = vx::EventType::Ingame_Event;
		evt.code = (u32)IngameEvent::Physx_AddedActor;
		evt.arg1.ptr = data;

		auto evtManager = Locator::getEventManager();
		evtManager->addEvent(evt);
	}
}

vx::StringID PhysicsAspect::raycast_static(const vx::float3 &origin, const vx::float3 &dir, f32 maxDistance, vx::float3* hitPosition, f32* distance) const
{
	physx::PxVec3 rayOrigin(origin.x, origin.y, origin.z);                 // [in] Ray origin
	physx::PxVec3 unitDir(dir.x, dir.y, dir.z);                // [in] Normalized ray direction

	return raycast_static(rayOrigin, unitDir, maxDistance, hitPosition, distance);
}

vx::StringID PhysicsAspect::raycast_static(const vx::float3 &origin, const vx::float4a &dir, f32 maxDistance, vx::float3* hitPosition, f32* distance) const
{
	physx::PxVec3 rayOrigin(origin.x, origin.y, origin.z);                 // [in] Ray origin
	physx::PxVec3 unitDir(dir.x, dir.y, dir.z);                // [in] Normalized ray direction

	return raycast_static(rayOrigin, unitDir, maxDistance, hitPosition, distance);
}

vx::StringID PhysicsAspect::raycast_static(const vx::float4a &origin, const vx::float4a &dir, f32 maxDistance, vx::float3* hitPosition, f32* distance) const
{
	physx::PxVec3 rayOrigin(origin.x, origin.y, origin.z);                 // [in] Ray origin
	physx::PxVec3 unitDir(dir.x, dir.y, dir.z);                // [in] Normalized ray direction
	
	return raycast_static(rayOrigin, unitDir, maxDistance, hitPosition, distance);
}

vx::StringID PhysicsAspect::raycast_static(const physx::PxVec3 &origin, const physx::PxVec3 &dir, f32 maxDistance, vx::float3* hitPosition, f32* distance) const
{
	physx::PxRaycastBuffer hit;
	physx::PxQueryFilterData filterData(physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC);

	auto unitDir = dir;
	unitDir.normalize();

	m_pScene->raycast(origin, unitDir, maxDistance, hit, physx::PxHitFlag::eDEFAULT, filterData);

	u8 result = hit.hasBlock;
	vx::StringID sid;
	if (result != 0)
	{
		hitPosition->x = hit.block.position.x;
		hitPosition->y = hit.block.position.y;
		hitPosition->z = hit.block.position.z;
		*distance = hit.block.distance;

		auto pActor = hit.block.actor;
		auto sidptr = (vx::StringID*)&pActor->userData;
		sid.value = sidptr->value;
	}

	return sid;
}

physx::PxConvexMesh* PhysicsAspect::getConvexMesh(const vx::StringID &sid, const vx::MeshFile &meshFile)
{
	physx::PxConvexMesh* result = nullptr;

	auto it = m_physxConvexMeshes.find(sid);
	if (it == m_physxConvexMeshes.end())
	{
		result = processMeshConvex(meshFile);

		m_physxConvexMeshes.insert(sid, result);
		m_physxMeshTypes.insert(sid, PhsyxMeshType::Convex);
	}
	else
	{
		result = *it;
	}

	VX_ASSERT(result != nullptr);
	return result;
}

physx::PxTriangleMesh* PhysicsAspect::getTriangleMesh(const vx::StringID &sid, const vx::MeshFile &meshFile)
{
	physx::PxTriangleMesh* result = nullptr;

	auto it = m_physxMeshes.find(sid);
	if (it == m_physxMeshes.end())
	{
		result = processTriangleMesh(meshFile);

		m_physxMeshes.insert(sid, result);
		m_physxMeshTypes.insert(sid, PhsyxMeshType::Triangle);
	}
	else
	{
		result = *it;
	}

	VX_ASSERT(result != nullptr);
	return result;
}

void PhysicsAspect::addMeshInstance(const MeshInstance &meshInstance)
{
	auto instanceSid = meshInstance.getNameSid();
	auto meshSid = meshInstance.getMeshSid();
	auto instanceTransform = meshInstance.getTransform();

	auto qRotation = vx::loadFloat4(instanceTransform.m_qRotation);

	physx::PxTransform transform;
	transform.p.x = instanceTransform.m_translation.x;
	transform.p.y = instanceTransform.m_translation.y;
	transform.p.z = instanceTransform.m_translation.z;
	_mm_storeu_ps(&transform.q.x, qRotation);

	assert(transform.isValid());

	auto &material = meshInstance.getMaterial();
	auto materialSid = (*material).getSid();
	auto itPhysxMaterial = m_physxMaterials.find(materialSid);
	if (itPhysxMaterial == m_physxMaterials.end())
	{
		auto pMaterial = m_pPhysics->createMaterial((*material).getStaticFriction(), (*material).getDynamicFriction(), (*material).getRestitution());
		VX_ASSERT(pMaterial);
		itPhysxMaterial = m_physxMaterials.insert(materialSid, pMaterial);
	}
	VX_ASSERT(itPhysxMaterial != m_physxMaterials.end());
	auto pmat = *itPhysxMaterial;

	auto itType = m_physxMeshTypes.find(meshSid);

	auto fileAspect = Locator::getFileAspect();
	auto meshFile = fileAspect->getMesh(meshSid);
	auto physxType = meshFile->getPhysxMeshType();

	physx::PxShape* shape = nullptr;
	switch (physxType)
	{
	case PhsyxMeshType::Triangle:
	{
		auto triangleMesh = getTriangleMesh(meshSid, (*meshFile));
		shape = m_pPhysics->createShape(physx::PxTriangleMeshGeometry(triangleMesh), *pmat);
	}break;
	case PhsyxMeshType::Convex:
	{
		auto convexMesh = getConvexMesh(meshSid, (*meshFile));
		shape = m_pPhysics->createShape(physx::PxConvexMeshGeometry(convexMesh), *pmat);
	}break;
	default:
		break;
	}

	PhysxRigidBodyType type = meshInstance.getRigidBodyType();
	if (type == PhysxRigidBodyType::Static)
	{
		addStaticMeshInstance(transform, *shape, instanceSid);
	}
	else
	{
		addDynamicMeshInstance(transform, *shape, instanceSid);
	}
}

void PhysicsAspect::addStaticMeshInstance(const physx::PxTransform &transform, physx::PxShape &shape, const vx::StringID &instanceSid)
{
	auto pRigidStatic = m_pPhysics->createRigidStatic(transform);
	pRigidStatic->attachShape(shape);

	auto sidptr = (vx::StringID*)&pRigidStatic->userData;
	sidptr->value = instanceSid.value;

	m_staticMeshInstances.insert(instanceSid, pRigidStatic);

	m_pScene->addActor(*pRigidStatic);
}

void PhysicsAspect::addDynamicMeshInstance(const physx::PxTransform &transform, physx::PxShape &shape, const vx::StringID &instanceSid)
{
	auto rigidDynamic = m_pPhysics->createRigidDynamic(transform);
	rigidDynamic->attachShape(shape);

	auto sidptr = (vx::StringID*)&rigidDynamic->userData;
	sidptr->value = instanceSid.value;

	m_dynamicMeshInstances.insert(instanceSid, rigidDynamic);

	m_pScene->addActor(*rigidDynamic);
}

void PhysicsAspect::processScene(const Scene* ptr)
{
	m_pScene->lockWrite();

	auto pScene = (Scene*)ptr;

	auto &meshes = pScene->getMeshes();
	auto keys = meshes.keys();

	for (auto i = 0u; i < meshes.size(); ++i)
	{
		bool isTriangleMesh = false;
		auto meshFile = meshes[i];
		if (!addMesh(keys[i], (*meshFile)))
		{
			VX_ASSERT(false);
		}
	}

	auto numInstances = pScene->getMeshInstanceCount();
	auto pMeshInstances = pScene->getMeshInstances();

	auto sceneMaterials = pScene->getMaterials();
	auto materialCount = pScene->getMaterialCount();
	for (auto i = 0u; i < materialCount; ++i)
	{
		auto &currentMaterial = sceneMaterials[i];

		auto sid = (*currentMaterial).getSid();
		auto pMaterial = m_pPhysics->createMaterial((*currentMaterial).getStaticFriction(), (*currentMaterial).getDynamicFriction(), (*currentMaterial).getRestitution());
		assert(pMaterial);
		m_physxMaterials.insert(sid, pMaterial);
	}

	for (auto i = 0u; i < numInstances; ++i)
	{
		auto &instance = pMeshInstances[i];
		addMeshInstance(instance);
	}

	m_pScene->unlockWrite();
}

physx::PxTriangleMesh* PhysicsAspect::processTriangleMesh(const vx::MeshFile &mesh)
{
	physx::PxDefaultMemoryInputData readBuffer((physx::PxU8*)mesh.getPhysxData(), mesh.getPhysxDataSize());

	return m_pPhysics->createTriangleMesh(readBuffer);
}

physx::PxConvexMesh* PhysicsAspect::processMeshConvex(const vx::MeshFile &mesh)
{
	physx::PxDefaultMemoryInputData readBuffer((physx::PxU8*)mesh.getPhysxData(), mesh.getPhysxDataSize());

	return m_pPhysics->createConvexMesh(readBuffer);
}

bool PhysicsAspect::addMesh(const vx::StringID &sid, const vx::MeshFile &meshFile)
{
	bool result = false;

	auto physxType = meshFile.getPhysxMeshType();
	if (physxType == PhsyxMeshType::Triangle)
	{
		auto resultTriangleMesh = processTriangleMesh(meshFile);
		if (resultTriangleMesh)
		{
			m_physxMeshes.insert(sid, resultTriangleMesh);
			result = true;
			m_physxMeshTypes.insert(sid, PhsyxMeshType::Triangle);
		}
	}
	else
	{
		auto resultConvexMesh = processMeshConvex(meshFile);
		if (resultConvexMesh)
		{
			m_physxConvexMeshes.insert(sid, resultConvexMesh);
			result = true;

			m_physxMeshTypes.insert(sid, PhsyxMeshType::Convex);
		}
	}

	return result;
	
}

physx::PxController* PhysicsAspect::createActor(const vx::StringID &mesh, const vx::float3 &translation)
{
	auto it = m_physxMeshes.find(mesh);

	auto bounds = (*it)->getLocalBounds();
	auto extend = bounds.getExtents();

	return createActor(translation, extend.z);
}

physx::PxController* PhysicsAspect::createActor(const vx::float3 &translation, f32 height)
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

void PhysicsAspect::move(const vx::float4a &velocity, f32 dt, physx::PxController* pController)
{
	physx::PxControllerFilters filters;
	//filters.

	physx::PxVec3 v;
	v.x = velocity.x;
	v.y = velocity.y;
	v.z = velocity.z;

	pController->move(v, 0.0001f, dt, filters);
}

physx::PxRigidStatic* PhysicsAspect::getStaticMesh(const vx::StringID &sid)
{
	physx::PxRigidStatic* result = nullptr;

	auto it = m_staticMeshInstances.find(sid);
	if (it != m_staticMeshInstances.end())
	{
		result = *it;
	}

	return result;
}

physx::PxRigidDynamic* PhysicsAspect::getDynamicMesh(const vx::StringID &sid)
{
	physx::PxRigidDynamic* result = nullptr;

	auto it = m_dynamicMeshInstances.find(sid);
	if (it != m_dynamicMeshInstances.end())
	{
		result = *it;
	}

	return result;
}