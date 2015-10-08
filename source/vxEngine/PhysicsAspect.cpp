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
#include <vxEngineLib/Message.h>
#include <vxEngineLib/MessageTypes.h>
#include <vxEngineLib/MeshInstance.h>
#include <vxEngineLib/Locator.h>
#include <vxEngineLib/Material.h>
#include <vxResourceAspect/ResourceAspect.h>
#include <vxEngineLib/Scene.h>
#include <vxEngineLib/MeshFile.h>
#include <vxEngineLib/FileMessage.h>
#include <vxEngineLib/IngameMessage.h>
#include <vxEngineLib/CreateActorData.h>
#include <vxEngineLib/MessageManager.h>
#include <vxEngineLib/CreateDynamicMeshData.h>
#include <vxEngineLib/Joint.h>
#include <vxEngineLib/Event.h>
#include <Windows.h>

UserErrorCallback PhysicsAspect::s_defaultErrorCallback{};
physx::PxDefaultAllocator PhysicsAspect::s_defaultAllocatorCallback{};

void UserErrorCallback::reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
{
	// error processing implementation
	//...

	printf("Physx Error: %d, %s %s %d\n", code, message, file, line);
}

class MySimulationCallback : public physx::PxSimulationEventCallback
{
public:
	MySimulationCallback()
	{

	}

	~MySimulationCallback()
	{

	}

	void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count)
	{

	}

	void onWake(physx::PxActor** actors, physx::PxU32 count) override
	{

	}

	void onSleep(physx::PxActor** actors, physx::PxU32 count) override
	{

	}

	void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override
	{
		printf("contact\n");
	}

	void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override
	{
		printf("trigger\n");
	}
};

class FilterHumanCallback : public physx::PxQueryFilterCallback
{
	physx::PxRigidDynamic* m_humanActor;

public:
	explicit FilterHumanCallback(physx::PxRigidDynamic* human) :m_humanActor(human) {}

	~FilterHumanCallback()
	{

	}

	physx::PxQueryHitType::Enum preFilter(
		const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags) override
	{
		if (m_humanActor == actor)
		{
			return physx::PxQueryHitType::eNONE;
		}
		else
		{
			return physx::PxQueryHitType::eBLOCK;
		}
	}

	physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit)
	{
		return physx::PxQueryHitType::eBLOCK;
	}
};

namespace PhysicsAspectCpp
{
	void copy(const vx::Transform &transform, physx::PxTransform* physxTransform)
	{
		physxTransform->p.x = transform.m_translation.x;
		physxTransform->p.y = transform.m_translation.y;
		physxTransform->p.z = transform.m_translation.z;

		physxTransform->q.x = transform.m_qRotation.x;
		physxTransform->q.y = transform.m_qRotation.y;
		physxTransform->q.z = transform.m_qRotation.z;
		physxTransform->q.w = transform.m_qRotation.w;
	}

	void copy(const vx::float3 &src, physx::PxVec3* dst)
	{
		dst->x = src.x;
		dst->y = src.y;
		dst->z = src.z;
	}

	void copy(const vx::float4 &src, physx::PxQuat* dst)
	{
		dst->x = src.x;
		dst->y = src.y;
		dst->z = src.z;
		dst->w = src.w;
	}
}

class MyHitReportCallback : public physx::PxUserControllerHitReport
{
public:
	void onShapeHit(const physx::PxControllerShapeHit& hit) override
	{
		auto type = hit.actor->getType();

		if (physx::PxActorType::eRIGID_DYNAMIC == type)
		{
			physx::PxRigidDynamic* ptr = (physx::PxRigidDynamic*)hit.actor;

			//printf("%f %f %f\n",hit.dir.x, hit.dir.y, hit.dir.z);
			ptr->addForce(hit.dir * 10);
		}
	}

	void onControllerHit(const physx::PxControllersHit& hit) override
	{
	}

	void onObstacleHit(const physx::PxControllerObstacleHit& hit) override
	{
	}
};

PhysicsAspect::PhysicsAspect()
	:m_pScene(nullptr),
	m_pControllerManager(nullptr),
	m_pActorMaterial(nullptr),
	m_pPhysics(nullptr),
	m_humanActor(nullptr),
	m_physxMeshes(),
	m_physxMaterials(),
	m_staticMeshInstances(),
	m_meshInstances(),
	m_pFoundation(nullptr),
	m_pCpuDispatcher(nullptr),
	m_pCooking(nullptr),
	m_callback(nullptr),
	m_mySimCallback(nullptr),
	m_connection(nullptr)
{
	m_flag.store(0);
	m_currentWriterTid.store(0);
	m_writerCount.store(0);
}

PhysicsAspect::~PhysicsAspect()
{
	if (m_callback)
	{
		delete(m_callback);
		m_callback = nullptr;
	}

	if (m_mySimCallback)
	{
		delete(m_mySimCallback);
		m_mySimCallback = nullptr;
	}
}

bool PhysicsAspect::initialize(vx::TaskManager* taskManager)
{
	m_pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, s_defaultAllocatorCallback,
		s_defaultErrorCallback);

	if (!m_pFoundation)
	{
		return false;
	}

	bool recordMemoryAllocations = true;

	physx::PxTolerancesScale toleranceScale;
	m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pFoundation,
		toleranceScale, recordMemoryAllocations);
	if (!m_pPhysics)
	{
		return false;
	}

	m_pCooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_pFoundation, physx::PxCookingParams(toleranceScale));
	if (!m_pCooking)
	{
		return false;
	}

	m_cpuDispatcher.initialize(taskManager);

	m_callback = new MyHitReportCallback();

	physx::PxSceneDesc sceneDesc(toleranceScale);
	sceneDesc.cpuDispatcher = &m_cpuDispatcher;
	sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.simulationEventCallback = m_mySimCallback;
	//sceneDesc.flags |= physx::PxSceneFlag::eENABLE_KINEMATIC_PAIRS;

	if (!sceneDesc.filterShader)
		sceneDesc.filterShader = &physx::PxDefaultSimulationFilterShader;

	m_pScene = m_pPhysics->createScene(sceneDesc);
	if (!m_pScene)
	{
		return false;
	}

	auto pvd = m_pPhysics->getPvdConnectionManager();
	if (pvd)
	{
		// setup connection parameters
		const char*     pvd_host_ip = "127.0.0.1";  // IP of the PC which is running PVD
		int             port = 5425;         // TCP port to connect to, where PVD is listening
		unsigned int    timeout = 100;          // timeout in milliseconds to wait for PVD to respond,
												// consoles and remote PCs need a higher timeout.
		physx::PxVisualDebuggerConnectionFlags connectionFlags = physx::PxVisualDebuggerExt::getAllConnectionFlags();
		m_connection = physx::PxVisualDebuggerExt::createConnection(pvd,
			pvd_host_ip, port, timeout, connectionFlags);
	}

	m_pActorMaterial = m_pPhysics->createMaterial(1, 1, 0.1f);

	m_pControllerManager = PxCreateControllerManager(*m_pScene);

	m_evtFetch = Event::createEvent();

	m_evtFetch.setStatus(EventStatus::Queued);

	return true;
}

void PhysicsAspect::shutdown()
{
	if (m_pScene)
	{
		m_pScene->fetchResults(true);
	}

	m_joints.clear();

	if (m_connection)
	{
		m_connection->disconnect();
		m_connection->release();
		m_connection = nullptr;
	}

	if (m_pControllerManager)
	{
		m_pControllerManager->release();
		m_pControllerManager = nullptr;
	}

	if (m_pScene)
	{
		m_pScene->release();
		m_pScene = nullptr;
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
	physx::PxU32 errorCode = 0;
	if (!m_pScene->fetchResults(true, &errorCode))
	{
		printf("error: %u\n", errorCode);
	}
	m_evtFetch.setStatus(EventStatus::Complete);
	m_flag.store(0);
}

void PhysicsAspect::update(const f32 dt)
{
	if (!m_blockEvents.empty())
	{
		for (auto &it : m_blockEvents)
		{
			auto status = it.getStatus();
			if (status == EventStatus::Running)
			{
				while (it.getStatus() != EventStatus::Complete)
					;
			}
		}
		m_blockEvents.clear();
	}

	m_flag.store(1);
	m_evtFetch.setStatus(EventStatus::Queued);
	m_pScene->simulate(dt);
}

void PhysicsAspect::addBlockEvent(const Event &evt)
{
	m_blockEvents.push_back(evt);
}

void PhysicsAspect::handleMessage(const vx::Message &evt)
{
	switch (evt.type)
	{
	case(vx::MessageType::Ingame) :
		handleIngameMessage(evt);
		break;
	case(vx::MessageType::File) :
		handleFileEvent(evt);
		break;
	default:
		break;
	}
}

void PhysicsAspect::handleFileEvent(const vx::Message &evt)
{
	auto fileEvent = (vx::FileMessage)evt.code;
	switch (fileEvent)
	{
	case vx::FileMessage::Scene_Loaded:
		processScene((Scene*)evt.arg2.ptr);
		break;
	default:
		break;
	}
}

void PhysicsAspect::handleIngameMessage(const vx::Message &evt)
{
	auto ingameMessage = (IngameMessage)evt.code;

	switch (ingameMessage)
	{
	case IngameMessage::Physx_AddDynamicMesh:
	{
		auto data = (CreateDynamicMeshData*)evt.arg1.ptr;

		auto &meshInstance = *(data->getMeshInstance());
		physx::PxRigidDynamic* rigidDynamic = nullptr;
		addMeshInstanceImpl(meshInstance, (void**)&rigidDynamic);
		data->setRigidDynamic(rigidDynamic);

		vx::Message evt;
		evt.type = vx::MessageType::Ingame;
		evt.code = (u32)IngameMessage::Physx_AddedDynamicMesh;
		evt.arg1.ptr = data;

		auto evtManager = Locator::getMessageManager();
		evtManager->addMessage(evt);
	}break;
	case IngameMessage::Physx_AddStaticMesh:
	{
		auto instance = (MeshInstance*)evt.arg1.ptr;
		addMeshInstanceImpl(*instance, nullptr);
	}break;
	default:
		break;
	}
}

bool PhysicsAspect::raycast_staticDynamic(const vx::float3 &origin, const vx::float3 &dir, f32 maxDistance, PhysicsHitData* hitData) const
{
	physx::PxVec3 rayOrigin(origin.x, origin.y, origin.z);                 // [in] Ray origin
	physx::PxVec3 unitDir(dir.x, dir.y, dir.z);                // [in] Normalized ray direction
	physx::PxQueryFilterData filterData(physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC);

	return raycast(rayOrigin, unitDir, filterData, maxDistance, hitData);
}

bool PhysicsAspect::raycast_staticDynamic(const vx::float3 &origin, const vx::float4a &dir, f32 maxDistance, PhysicsHitData* hitData) const
{
	physx::PxVec3 rayOrigin(origin.x, origin.y, origin.z);                 // [in] Ray origin
	physx::PxVec3 unitDir(dir.x, dir.y, dir.z);                // [in] Normalized ray direction
	physx::PxQueryFilterData filterData(physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC);

	return raycast(rayOrigin, unitDir, filterData, maxDistance, hitData);
}

bool PhysicsAspect::raycast_staticDynamic(const vx::float4a &origin, const vx::float4a &dir, f32 maxDistance, PhysicsHitData* hitData) const
{
	physx::PxVec3 rayOrigin(origin.x, origin.y, origin.z);                 // [in] Ray origin
	physx::PxVec3 unitDir(dir.x, dir.y, dir.z);                // [in] Normalized ray direction
	physx::PxQueryFilterData filterData(physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC);
	
	return raycast(rayOrigin, unitDir, filterData, maxDistance, hitData);
}

bool PhysicsAspect::raycastDynamic(const vx::float3 &origin, const vx::float3 &dir, f32 maxDistance, PhysicsHitData* hitData) const
{
	physx::PxVec3 rayOrigin(origin.x, origin.y, origin.z);                 // [in] Ray origin
	physx::PxVec3 unitDir(dir.x, dir.y, dir.z);                // [in] Normalized ray direction
	physx::PxQueryFilterData filterData(physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::ePREFILTER);

	return raycastNoPlayer(rayOrigin, unitDir, filterData, maxDistance, hitData);
}

bool PhysicsAspect::raycast(const physx::PxVec3 &origin, const physx::PxVec3 &dir, const physx::PxQueryFilterData &filterData, f32 maxDistance, PhysicsHitData* hitData) const
{
	physx::PxRaycastBuffer hit;

	auto unitDir = dir;
	unitDir.normalize();

	bool result = false;
	if (m_pScene->raycast(origin, unitDir, maxDistance, hit, physx::PxHitFlag::eDEFAULT, filterData))
	{
		result = true;

		if (hit.hasBlock != 0)
		{
			auto pActor = hit.block.actor;
			auto sidptr = (vx::StringID*)&pActor->userData;

			hitData->hitPosition.x = hit.block.position.x;
			hitData->hitPosition.y = hit.block.position.y;
			hitData->hitPosition.z = hit.block.position.z;
			hitData->distance = hit.block.distance;
			hitData->actor = hit.block.actor;
			hitData->sid = sidptr->value;
		}
	}

	return result;
}

bool PhysicsAspect::raycastNoPlayer(const physx::PxVec3 &origin, const physx::PxVec3 &dir, const physx::PxQueryFilterData &filterData, f32 maxDistance, PhysicsHitData* hitData) const
{
	physx::PxRaycastBuffer hit;

	auto unitDir = dir;
	unitDir.normalize();

	FilterHumanCallback callback(m_humanActor);

	bool result = false;
	if (m_pScene->raycast(origin, unitDir, maxDistance, hit, physx::PxHitFlag::eDEFAULT, filterData, &callback))
	{
		result = true;

		if (hit.hasBlock != 0)
		{
			auto pActor = hit.block.actor;
			auto sidptr = (vx::StringID*)&pActor->userData;

			hitData->hitPosition.x = hit.block.position.x;
			hitData->hitPosition.y = hit.block.position.y;
			hitData->hitPosition.z = hit.block.position.z;
			hitData->distance = hit.block.distance;
			hitData->actor = hit.block.actor;
			hitData->sid = sidptr->value;
		}
	}

	return result;
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

void PhysicsAspect::addMeshInstanceImpl(const MeshInstance &meshInstance, void** outData)
{
	auto instanceSid = meshInstance.getNameSid();
	auto meshSid = meshInstance.getMeshSid();
	auto instanceTransform = meshInstance.getTransform();

	auto qRotation = vx::loadFloat4(&instanceTransform.m_qRotation);

	physx::PxTransform transform;
	transform.p.x = instanceTransform.m_translation.x;
	transform.p.y = instanceTransform.m_translation.y;
	transform.p.z = instanceTransform.m_translation.z;
	_mm_storeu_ps(&transform.q.x, qRotation);

	VX_ASSERT(transform.isValid());

	auto material = meshInstance.getMaterial();
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

	auto resourceAspect = Locator::getResourceAspect();
	auto meshFile = resourceAspect->getMesh(meshSid);
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
		VX_ASSERT(false);
		break;
	}

	PhysxRigidBodyType type = meshInstance.getRigidBodyType();
	if (type == PhysxRigidBodyType::Static)
	{
		addStaticMeshInstance(transform, *shape, instanceSid);
	}
	else
	{
		addDynamicMeshInstance(transform, *shape, instanceSid, outData);
	}

	m_meshInstances.insert(instanceSid, type);
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

void PhysicsAspect::addDynamicMeshInstance(const physx::PxTransform &transform, physx::PxShape &shape, const vx::StringID &instanceSid, void** outData)
{
	auto rigidDynamic = m_pPhysics->createRigidDynamic(transform);
	rigidDynamic->attachShape(shape);

	auto sidptr = (vx::StringID*)&rigidDynamic->userData;
	sidptr->value = instanceSid.value;

	m_dynamicMeshInstances.insert(instanceSid, rigidDynamic);

	*outData = rigidDynamic;

	m_pScene->addActor(*rigidDynamic);
}

void PhysicsAspect::processScene(const Scene* pScene)
{
	lockSceneWrite();

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

	unlockSceneWrite();

	vx::Message e;
	e.arg1.ptr = (void*)pScene;
	e.type = vx::MessageType::Ingame;
	e.code = (u32)IngameMessage::Physx_CreatedScene;

	auto pEvtManager = Locator::getMessageManager();
	pEvtManager->addMessage(e);
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

/*physx::PxController* PhysicsAspect::createActor(const vx::StringID &mesh, const vx::float3 &translation)
{
	if (m_flag.load() != 0)
	{
		puts("PhysicsAspect::createActor");
		VX_ASSERT(false);
	}

	auto it = m_physxMeshes.find(mesh);

	auto bounds = (*it)->getLocalBounds();
	auto extend = bounds.getExtents();

	return createActor(translation, extend.z);
}*/

physx::PxController* PhysicsAspect::createActor(const vx::float3 &translation, f32 height, f32 radius)
{
	if (m_flag.load() != 0)
	{
		puts("PhysicsAspect::createActor");
		VX_ASSERT(false);
	}

	physx::PxCapsuleControllerDesc desc;
	desc.height = height;
	desc.radius = radius;
	desc.position.x = translation.x;
	desc.position.y = translation.y;
	desc.position.z = translation.z;
	desc.stepOffset = 0.2f;
	desc.material = m_pActorMaterial;
	//desc.behaviorCallback = ;
	desc.reportCallback = m_callback;

	VX_ASSERT(desc.isValid());

	lockSceneWrite();

	auto p = m_pControllerManager->createController(desc);

	unlockSceneWrite();

	VX_ASSERT(p);

	return p;
}

void PhysicsAspect::move(const vx::float4a &velocity, f32 dt, physx::PxController* pController)
{
	if (m_flag.load() != 0)
	{
		puts("PhysicsAspect::move");
		VX_ASSERT(false);
	}

	physx::PxControllerFilters filters;
	//filters.

	physx::PxVec3 v;
	v.x = velocity.x;
	v.y = velocity.y;
	v.z = velocity.z;

	lockSceneWrite();

	pController->move(v, 0.0001f, dt, filters);

	unlockSceneWrite();
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

physx::PxRigidActor* PhysicsAspect::getRigidActor(const vx::StringID &sid, PhysxRigidBodyType* outType)
{
	physx::PxRigidActor* actor = nullptr;

	auto it = m_meshInstances.find(sid);
	if (it != m_meshInstances.end())
	{
		auto type = *it;
		switch (type)
		{
		case PhysxRigidBodyType::Static:
		{
			auto itActor = m_staticMeshInstances.find(sid);
			actor = *itActor;
			*outType = PhysxRigidBodyType::Static;
		}break;
		case PhysxRigidBodyType::Dynamic:
		{
			auto itActor = m_dynamicMeshInstances.find(sid);
			actor = *itActor;
			*outType = PhysxRigidBodyType::Dynamic;
		}break;
		default:
			break;
		}
	}

	return actor;
}

void PhysicsAspect::lockSceneWrite()
{
	u32 tid = GetCurrentThreadId();

	auto current = m_currentWriterTid.load();
//	printf("%u\n", current);
	u32 expected = 0;

	while (!m_currentWriterTid.compare_exchange_weak(expected, tid))
	{
		expected = 0;
	}

	m_writerCount.fetch_add(1);
	m_pScene->lockWrite();
}

void PhysicsAspect::unlockSceneWrite()
{
	m_pScene->unlockWrite();
	auto oldSize = m_writerCount.fetch_sub(1);
	if (oldSize != 1)
	{
		puts("ERROR PhysicsAspect::unlockSceneWrite");
	}

	if (oldSize == 1)
	{
		m_currentWriterTid.store(0);
	}
}

physx::PxJoint* PhysicsAspect::createJoint(const Joint &joint)
{
	if (m_flag.load() != 0)
	{
		puts("PhysicsAspect::createJoint");
		//VX_ASSERT(false);
	}

	physx::PxRevoluteJoint* result = nullptr;

	PhysxRigidBodyType type0, type1;

	auto actor0 = getRigidActor(joint.sid0, &type0);
	auto actor1 = getRigidActor(joint.sid1, &type1);
	if (actor0 || actor1)
	{
		physx::PxTransform localFrame0, localFrame1;
		PhysicsAspectCpp::copy(joint.p0, &localFrame0.p);
		PhysicsAspectCpp::copy(joint.q0, &localFrame0.q);
		PhysicsAspectCpp::copy(joint.p1, &localFrame1.p);
		PhysicsAspectCpp::copy(joint.q1, &localFrame1.q);

		lockSceneWrite();

		auto ptr = physx::PxRevoluteJointCreate(*m_pPhysics, actor0, localFrame0, nullptr, localFrame1);

		if (ptr != nullptr)
		{
			if (joint.limitEnabled != 0)
			{
				auto limitPair = physx::PxJointAngularLimitPair(joint.limit.x, joint.limit.y);

				ptr->setLimit(limitPair);
				ptr->setRevoluteJointFlag(physx::PxRevoluteJointFlag::eLIMIT_ENABLED, true);
			}

			m_joints.push_back(result);
		}

		unlockSceneWrite();

		result = ptr;
	}

	return result;
}

physx::PxJoint* PhysicsAspect::createSphericalJoint(physx::PxRigidActor* actor)
{
	if (m_flag.load() != 0)
	{
		puts("PhysicsAspect::createSphericalJoint");
		VX_ASSERT(false);
	}

	physx::PxTransform localFrame0;
	localFrame0.p = {0, 0, 0};
	localFrame0.q =  { 0.000000000f, 0.f, 0.f, 1.f };

	auto transform = actor->getGlobalPose();

	physx::PxTransform localFrame1;
	localFrame1.p = transform.p;
	localFrame1.q = { 0.000000000f, 0.f, 0.f, 1.f };

	lockSceneWrite();

	physx::PxSphericalJoint* result = physx::PxSphericalJointCreate(*m_pPhysics, actor, localFrame0, nullptr, localFrame1);

	m_pScene->unlockWrite();

	//physx::PxJointLimitCone limit(1.5f, 2.4f);
	//result->setLimitCone(limit);
	//result->setSphericalJointFlags(physx::PxSphericalJointFlag::eLIMIT_ENABLED);

	return result;
}

physx::PxJoint* PhysicsAspect::createFixedJoint(physx::PxRigidActor* actor)
{
	if (m_flag.load() != 0)
	{
		puts("PhysicsAspect::createFixedJoint");
		VX_ASSERT(false);
	}

	physx::PxJoint* result = nullptr;

	physx::PxTransform localFrame0;
	localFrame0.p = { 0.0f, 0.0f, 0 };
	localFrame0.q = { 0.000000000f, 0.f, 0.f, 1.f };

	auto transform = actor->getGlobalPose();
	//physx::PxRigidDynamic* rigid = (physx::PxRigidDynamic*)actor;
	//rigid->

	physx::PxTransform localFrame1;
	localFrame1.p = transform.p;
	localFrame1.q = { 0.000000000f, 0.f, 0.f, 1.f };

	//printf("%f %f\n", localFrame1.p.x, localFrame1.p.z);

	//PhysicsAspectCpp::copy(joint.p1, &localFrame1.p);
	//PhysicsAspectCpp::copy(joint.q1, &localFrame1.q);

	//result = physx::PxFixedJointCreate(*m_pPhysics, actor0, localFrame0, nullptr, localFrame1);

	//auto ptr = physx::PxD6JointCreate(*m_pPhysics, actor0, localFrame0, nullptr, localFrame1);

	lockSceneWrite();

	result = physx::PxFixedJointCreate(*m_pPhysics, actor, localFrame0, nullptr, localFrame1);

	unlockSceneWrite();

	return result;
}

physx::PxJoint* PhysicsAspect::createD6Joint(physx::PxRigidActor* actor)
{
	if (m_flag.load() != 0)
	{
		puts("PhysicsAspect::createD6Joint");
		VX_ASSERT(false);
	}

	physx::PxTransform localFrame0;
	localFrame0.p = { 0, 0, 0 };
	localFrame0.q = { 0.000000000f, 0.f, 0.f, 1.f };

	auto transform = actor->getGlobalPose();

	physx::PxTransform localFrame1;
	localFrame1.p = transform.p;
	localFrame1.q = { 0.000000000f, 0.f, 0.f, 1.f };

	lockSceneWrite();

	physx::PxD6Joint* result = physx::PxD6JointCreate(*m_pPhysics, actor, localFrame0, nullptr, localFrame1);

	unlockSceneWrite();
	//result->setMotion(physx::PxD6Axis::eX, physx::PxD6Motion::eFREE);

	return result;
}

void PhysicsAspect::setHumanActor(physx::PxRigidDynamic* actor)
{
	m_humanActor = actor;
}