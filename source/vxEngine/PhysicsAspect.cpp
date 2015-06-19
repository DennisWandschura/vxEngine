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
#include <vxLib/Graphics/Mesh.h>
#include <vxEngineLib/Scene.h>
#include <vxEngineLib/EditorMeshInstance.h>
#include <Windows.h>
#include <vxEngineLib/Material.h>
#include <vxEngineLib/EditorScene.h>
#include <extensions/PxDefaultSimulationFilterShader.h>
#include "PhysicsDefines.h"
#include <vxEngineLib/Event.h>
#include <vxEngineLib/EventTypes.h>
#include "Locator.h"
#include <vxResourceAspect/FileAspect.h>
#include <vxEngineLib/MeshFile.h>
#include <vxEngineLib/FileEvents.h>

UserErrorCallback PhysicsAspect::s_defaultErrorCallback{};
physx::PxDefaultAllocator PhysicsAspect::s_defaultAllocatorCallback{};

void UserErrorCallback::reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
{
	// error processing implementation
	//...

	printf("Physx Error: %d, %s %s %d\n", code, message, file, line);
}

PhysicsAspect::PhysicsAspect(FileAspect &fileAspect)
	:m_pScene(nullptr),
	m_pControllerManager(nullptr),
	m_fileAspect(fileAspect),
	m_pActorMaterial(nullptr),
	m_pPhysics(nullptr),
	m_mutex(),
	m_physxMeshes(),
	m_physxMaterials(),
	m_staticMeshInstances(),
	m_pFoundation(nullptr),
	m_pCpuDispatcher(nullptr),
	m_pCooking(nullptr)
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
		processScene(evt.arg2.ptr);
		break;
	default:
		break;
	}
}

vx::StringID PhysicsAspect::raycast_static(const vx::float4a &origin, const vx::float4a &dir, f32 maxDistance, vx::float3* hitPosition) const
{
	physx::PxVec3 rayOrigin(origin.x, origin.y, origin.z);                 // [in] Ray origin
	physx::PxVec3 unitDir(dir.x, dir.y, dir.z);                // [in] Normalized ray direction
	physx::PxRaycastBuffer hit;                 // [out] Raycast results

	// [in] Define filter for static objects only
	physx::PxQueryFilterData filterData(physx::PxQueryFlag::eSTATIC);

	//unitDir.normalize();

	m_pScene->raycast(rayOrigin, unitDir, maxDistance, hit, physx::PxHitFlag::eDEFAULT, filterData);

	u8 result = hit.hasBlock;
	vx::StringID sid;
	if (result != 0)
	{
		hitPosition->x = hit.block.position.x;
		hitPosition->y = hit.block.position.y;
		hitPosition->z = hit.block.position.z;

		auto pActor = hit.block.actor;
		auto sidptr = (vx::StringID*)&pActor->userData;
		sid.value = sidptr->value;
	}

	return sid;
}

bool PhysicsAspect::editorGetStaticMeshInstancePosition(const vx::StringID &sid, vx::float3* p) const
{
	bool result = false;

	auto it = m_staticMeshInstances.find(sid);
	if (it != m_staticMeshInstances.end())
	{
		physx::PxTransform transform = (*it)->getGlobalPose();

		p->x = transform.p.x;
		p->y = transform.p.y;
		p->z = transform.p.z;

		result = true;
	}

	return result;
}

void PhysicsAspect::editorSetStaticMeshInstanceTransform(const MeshInstance &meshInstance, const vx::StringID &sid)
{
	auto it = m_staticMeshInstances.find(sid);
	if (it != m_staticMeshInstances.end())
	{
		auto instanceTransform = meshInstance.getTransform();

		auto qRotation = vx::loadFloat4(instanceTransform.m_qRotation);

		physx::PxTransform transform;
		transform.p.x = instanceTransform.m_translation.x;
		transform.p.y = instanceTransform.m_translation.y;
		transform.p.z = instanceTransform.m_translation.z;
		_mm_storeu_ps(&transform.q.x, qRotation);

		(*it)->setGlobalPose(transform);
	}
}

void PhysicsAspect::editorAddMeshInstance(const MeshInstance &instance)
{
	addMeshInstance(instance);
}

void PhysicsAspect::editorSetStaticMeshInstanceMesh(const MeshInstance &meshInstance)
{
	auto sid = meshInstance.getNameSid();
	auto rigidStaticIt = m_staticMeshInstances.find(sid);

	auto meshSid = meshInstance.getMeshSid();
	auto newTriangleMeshIt = m_physxMeshes.find(meshSid);
	auto newConvexMeshIt = m_physxConvexMeshes.find(meshSid);

	bool foundMesh = newTriangleMeshIt != m_physxMeshes.end() &&
		newConvexMeshIt != m_physxConvexMeshes.end();

	bool isTriangleMesh = false;
	if (!foundMesh)
	{
		puts("process mesh begin");
		auto fileAspect = Locator::getFileAspect();
		auto pMeshFile = fileAspect->getMesh(meshSid);

		if (!processMesh(meshSid, pMeshFile, &isTriangleMesh))
		{
			printf("error processing mesh\n");
			VX_ASSERT(false);
		}

		newTriangleMeshIt = m_physxMeshes.find(meshSid);
		newConvexMeshIt = m_physxConvexMeshes.find(meshSid);

		puts("process mesh end");
	}

	auto &material = meshInstance.getMaterial();
	auto itPhysxMaterial = m_physxMaterials.find((*material).getSid());

	puts("create shape begin");
	physx::PxShape* newShape = nullptr;
	if (isTriangleMesh)
	{
		newShape = m_pPhysics->createShape(physx::PxTriangleMeshGeometry(*newTriangleMeshIt), *(*itPhysxMaterial));
	}
	else
	{
		VX_ASSERT(newConvexMeshIt != m_physxConvexMeshes.end());
		newShape = m_pPhysics->createShape(physx::PxConvexMeshGeometry(*newConvexMeshIt), *(*itPhysxMaterial));
	}
	puts("create shape end");

	auto shapeCount = (*rigidStaticIt)->getNbShapes();
	VX_ASSERT(shapeCount == 1);

	physx::PxShape* oldShape = nullptr;;
	(*rigidStaticIt)->getShapes(&oldShape, 1);

	puts("attach shape begin");
	(*rigidStaticIt)->attachShape(*newShape);
	(*rigidStaticIt)->detachShape(*oldShape);
	puts("end shape begin");
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
	auto itPhysxMaterial = m_physxMaterials.find((*material).getSid());
	auto pmat = *itPhysxMaterial;

	physx::PxShape* shape = nullptr;
	auto itPhysxTriangleMesh = m_physxMeshes.find(meshSid);
	if (itPhysxTriangleMesh != m_physxMeshes.end())
	{
		shape = m_pPhysics->createShape(physx::PxTriangleMeshGeometry(*itPhysxTriangleMesh), *pmat);
	}
	else
	{
		auto it = m_physxConvexMeshes.find(meshSid);
		VX_ASSERT(it != m_physxConvexMeshes.end());
		shape = m_pPhysics->createShape(physx::PxConvexMeshGeometry(*it), *pmat);
	}

	auto pRigidStatic = m_pPhysics->createRigidStatic(transform);
	pRigidStatic->attachShape(*shape);

	auto sidptr = (vx::StringID*)&pRigidStatic->userData;
	sidptr->value = instanceSid.value;

	m_staticMeshInstances.insert(instanceSid, pRigidStatic);

	m_pScene->addActor(*pRigidStatic);
}

void PhysicsAspect::processScene(const void* ptr)
{
	m_pScene->lockWrite();

#if _VX_EDITOR
	auto pScene = (Editor::Scene*)ptr;
#else
	auto pScene = (Scene*)ptr;
#endif

	auto &meshes = pScene->getMeshes();
	auto keys = meshes.keys();

	for (auto i = 0u; i < meshes.size(); ++i)
	{
		bool isTriangleMesh = false;
		auto pMeshFile = meshes[i];
		if (!processMesh(keys[i], pMeshFile, &isTriangleMesh))
		{
			VX_ASSERT(false);
		}
	}

	auto numInstances = pScene->getMeshInstanceCount();
#if _VX_EDITOR
	auto pMeshInstances = pScene->getMeshInstancesEditor();
#else
	auto pMeshInstances = pScene->getMeshInstances();
#endif

	auto &sceneMaterials = pScene->getMaterials();
	for (auto i = 0u; i < sceneMaterials.size(); ++i)
	{
		auto &currentMaterial = sceneMaterials[i];

		auto sid = sceneMaterials.keys()[i];
		auto pMaterial = m_pPhysics->createMaterial((*currentMaterial).getStaticFriction(), (*currentMaterial).getDynamicFriction(), (*currentMaterial).getRestitution());
		assert(pMaterial);
		m_physxMaterials.insert(sid, pMaterial);
	}

	for (auto i = 0u; i < numInstances; ++i)
	{
#if _VX_EDITOR
		addMeshInstance(pMeshInstances[i].getMeshInstance());
#else
		addMeshInstance(pMeshInstances[i]);
#endif
	}

	m_pScene->unlockWrite();
}

physx::PxTriangleMesh* PhysicsAspect::processTriangleMesh(const vx::MeshFile* pMesh)
{
	physx::PxDefaultMemoryInputData readBuffer((physx::PxU8*)pMesh->getPhysxData(), pMesh->getPhysxDataSize());

	return m_pPhysics->createTriangleMesh(readBuffer);
}

physx::PxConvexMesh* PhysicsAspect::processMeshConvex(const vx::MeshFile* pMesh)
{
	physx::PxDefaultMemoryInputData readBuffer((physx::PxU8*)pMesh->getPhysxData(), pMesh->getPhysxDataSize());

	return m_pPhysics->createConvexMesh(readBuffer);
}

bool PhysicsAspect::processMesh(const vx::StringID &sid, const vx::MeshFile* pMeshFile, bool* isTriangleMesh)
{
	bool result = false;
	*isTriangleMesh = false;

	auto resultTriangleMesh = processTriangleMesh(pMeshFile);
	if (resultTriangleMesh)
	{
		// need to lock because we are modifying the vector
		std::lock_guard<std::mutex> guard(m_mutex);
		m_physxMeshes.insert(sid, resultTriangleMesh);
		result = true;
		*isTriangleMesh = true;
	}
	else
	{
		auto resultConvexMesh = processMeshConvex(pMeshFile);
		if (resultConvexMesh)
		{
			std::lock_guard<std::mutex> guard(m_mutex);
			m_physxConvexMeshes.insert(sid, resultConvexMesh);
			result = true;
		}
	}

	return result;
	
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

void PhysicsAspect::setPosition(const vx::float3 &position, physx::PxController* pController)
{
	pController->setPosition(physx::PxExtendedVec3(position.x, position.y, position.z));
}