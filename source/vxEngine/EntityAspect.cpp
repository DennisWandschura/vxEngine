#include "EntityAspect.h"
#include <vxLib/Allocator/StackAllocator.h>
#include <PhysX/characterkinematic/PxController.h>
#include "PhysicsAspect.h"
#include "Entity.h"
#include "ComponentPhysics.h"
#include "ComponentActor.h"
#include "ComponentInput.h"
#include "PhysicsDefines.h"
#include "Scene.h"
#include "Transform.h"
#include "RenderAspect.h"
#include "ComponentRender.h"
#include "EntityFactory.h"
#include "Actor.h"
#include "Event.h"
#include "EventTypes.h"
#include "Locator.h"
#include "enums.h"
#include "EventManager.h"
#include "InfluenceMap.h"
#include "NavGraph.h"
#include "EventsIngame.h"
#include <vxLib/ScopeGuard.h>
#include "Spawn.h"
#include "GpuFunctions.h"
#include "CreateActorData.h"
#include "EntityFactoryDescription.h"

namespace
{
	template<typename T>
	void createPool(U16 capacity, U8 alignment, vx::StackAllocator* pAllocator, Pool<T> *pPool)
	{
		pPool->initialize(pAllocator->allocate(sizeof(T) * capacity, alignment), capacity);
	}
}

EntityAspect::EntityAspect(PhysicsAspect &physicsAspect, FileAspect &fileAspect, RenderAspect &renderAspect)
	:m_playerController(&renderAspect),
	m_physicsAspect(physicsAspect),
	m_fileAspect(fileAspect),
	m_renderAspect(renderAspect)
{
}

bool EntityAspect::initialize(vx::StackAllocator* pAllocator)
{
	//createPool(g_maxEntities, 16, pAllocator, &m_poolPhysics);
	createPool(g_maxEntities, 16, pAllocator, &m_poolInput);
	createPool(g_maxEntities, 16, pAllocator, &m_poolRender);
	createPool(g_maxEntities, 16, pAllocator, &m_poolEntity);
	createPool(g_maxEntities, 16, pAllocator, &m_poolActor);

	const auto pathChunkSize = s_maxNavNodes * sizeof(vx::float3);
	const auto pathPoolSize = g_maxEntities * pathChunkSize;
	m_poolAllocatorPath = vx::PoolAllocator(pAllocator->allocate(pathPoolSize, 8), pathPoolSize, pathChunkSize, __alignof(vx::float3));

	m_allocator = vx::StackAllocator(pAllocator->allocate(1 KBYTE, 16), 1 KBYTE);

	return true;
}

void EntityAspect::shutdown()
{
	m_poolActor.release();
	m_poolEntity.release();
	m_poolRender.release();
	m_poolInput.release();
	//m_poolPhysics.release();
}

void EntityAspect::createComponentPhysics(const vx::float3 &position, U16 entityIndex, F32 height)
{
	auto pActor = m_physicsAspect.createActor(position, height);

	m_poolEntity[entityIndex].position = position;
	m_poolEntity[entityIndex].pRigidActor = pActor;
}

Component::Actor* EntityAspect::createComponentActor(U16 entityIndex, U16* actorIndex)
{
	auto pActor = m_poolActor.createEntry(actorIndex);
	pActor->entityIndex = entityIndex;

	/*Action* pEmptyAction = new Action();
	// trigger if distance to object is greater than 1.0f
	RaycastGreaterCondition* pRaycastCondition = new RaycastGreaterCondition(m_physicsAspect, pPhys, 3.0f, 1.0f);
	Transition *pTransition = new Transition(pRaycastCondition, );

	State* pInitialState = new State();
	pInitialState->addTransition(pTransition);
	pInitialState->setAction(pEmptyAction);

	pActor->stateMachine.setInitialState(pInitialState);

	pActor->stateMachine.addState();*/

	return pActor;
}

void EntityAspect::spawnPlayer(const vx::float3 &position, const Component::Physics &p)
{
	m_physicsAspect.setPosition(position, p.pRigidActor);
}

void EntityAspect::createPlayerEntity(const vx::float3 &position)
{
	//assert(!m_pPlayer);

	//
	//

	if (m_pPlayer == nullptr)
	{
		U16 entityIndex;
		m_pPlayer = m_poolEntity.createEntry(&entityIndex);

		auto pInput = m_poolInput.createEntry(&m_pPlayer->input);
		pInput->entityIndex = entityIndex;
		pInput->orientation.x = 0.0f;

		createComponentPhysics(position, entityIndex, g_heightStanding);
	}
}

void EntityAspect::createActorEntity(const vx::float3 &position, F32 height, U32 gpuIndex)
{
	U16 entityIndex;
	auto pEntity = m_poolEntity.createEntry(&entityIndex);
	auto pInput = m_poolInput.createEntry(&pEntity->input);
	pInput->entityIndex = entityIndex;

	createComponentPhysics(position, entityIndex, height);

	//auto &actors = m_pCurrentScene->getActors();
	//auto itActor = actors.find(actor);

	//vx::Transform transform;
	//transform.m_translation = position;

	//auto gpuIndex = m_renderAspect.addActorToBuffer(transform, itActor->mesh, itActor->material, m_pCurrentScene);
	auto pRender = m_poolRender.createEntry(&pEntity->render);
	pRender->entityIndex = entityIndex;
	pRender->gpuIndex = gpuIndex;

	auto pActor = createComponentActor(entityIndex, &pEntity->actor);

	EntityFactoryDescription desc;
	desc.entity = pEntity;
	desc.halfHeight = height * 0.5f;
	desc.navGraph = m_pNavGraph;
	desc.p = pActor;
	desc.pInput = pInput;

	EntityFactory::create(desc, &m_poolAllocatorPath);

	Event evt;
	evt.type = EventType::Ingame_Event;
	evt.code = (U32)IngameEvent::Created_Actor;
	evt.arg1 = pActor;

	auto pEvtManager = Locator::getEventManager();
	pEvtManager->addEvent(evt);
}

void EntityAspect::updateInput(F32 dt)
{
	const __m128 vGravity = { 0, g_gravity * dt, 0, 0 };
	//const __m128 vGravity = { 0, tmp, 0, 0 };
	// move x-axis, y-axis, z-axis
	//const __m128 move = { 0.2f, 0.0f, 0.2f, 0 };
	//const __m128 runOffset = { 0.1f, 0, 0.1f, 0 };
	//const __m128 vDt = { 1, dt, 1, 1 };
//	const DirectX::XMVECTORI32 mask = { -1, 0, -1 };

	auto p = m_poolInput.first();
	while (p != nullptr)
	{
		auto &entity = m_poolEntity[p->entityIndex];
	//	auto &physics = m_poolPhysics[entity.physics];

		/*F32 x_axis = ((I8)(p->action >> Component::Input::Action_Right) & 0x1) - ((I8)(p->action >> Component::Input::Action_Left) & 0x1);
		F32 z_axis = ((I8)(p->action >> Component::Input::Action_Backward) & 0x1) - ((I8)(p->action >> Component::Input::Action_Forward) & 0x1);
		F32 isRunning = ((U8)(p->action >> Component::Input::Action_Run) & 0x1);
		U8 crouch = ((U8)(p->action >> Component::Input::Action_Crouch) & 0x1);
		U8 isCrouching = ((U8)(p->state >> Component::Input::State_Crouch) & 0x1);
		// clear every input except status on jumping
		p->action = 0;
		//p->state = 0;

		__m128 vOffset = { x_axis, 0, z_axis, 0.0f };
		__m128 vSpeed = { isRunning, 0, isRunning, 0 };

		vSpeed = _mm_fmadd_ps(vSpeed, runOffset, move);

		vOffset = DirectX::XMVector3Rotate(vOffset, tmp);
		vOffset = _mm_and_ps(vOffset, mask);
		vOffset = DirectX::XMVector3Normalize(vOffset);

		vOffset = _mm_fmadd_ps(vOffset, vSpeed, gravity);
		vOffset = _mm_mul_ps(vOffset, vDt);

		// clamp looking up and looking down
		p->lookAngle.y = fmaxf(p->lookAngle.y, angleMin);
		p->lookAngle.y = fminf(p->lookAngle.y, angleMax);

		auto v = DirectX::XMQuaternionRotationRollPitchYaw(p->lookAngle.y, p->lookAngle.x, 0);

		_mm_storeu_ps(&physics.qorientation.x, v);

		if (isCrouching)
		{
			if (!crouch)
			{
				isCrouching = 0;
				physics.pRigidActor->resize(g_heightStanding);
			}
		}
		else
		{
			if (crouch)
			{
				isCrouching = 1;
				physics.pRigidActor->resize(g_heightCrouching);
			}
		}

		p->state = (isCrouching << Component::Input::State_Crouch);

		m_physicsAspect.move(vOffset, dt, physics.pRigidActor);*/

		entity.orientation = p->orientation;

		//__m128 vSpeed = { 0, 0, 0, 0 };
		//vSpeed = _mm_fmadd_ps(vSpeed, runOffset, move);

		__m128 vVelocity = { p->velocity.x, 0, p->velocity.z, 0.0f };
		//vOffset = _mm_fmadd_ps(vOffset, vSpeed, gravity);
		vVelocity = _mm_add_ps(vVelocity, vGravity);
		//vVelocity = _mm_mul_ps(vVelocity, vDt);

		m_physicsAspect.move(vVelocity, dt, entity.pRigidActor);

		p = m_poolInput.next_nocheck(p);
	}
}

void EntityAspect::updatePhysics_linear(F32 dt)
{
	UNREFERENCED_PARAMETER(dt);

	auto p = m_poolEntity.first();
	while (p != nullptr)
	{
		auto footPosition = p->pRigidActor->getFootPosition();

		auto position = p->pRigidActor->getPosition();
		p->position.x = position.x;
		p->position.y = position.y;
		p->position.z = position.z;
		p->footPositionY = footPosition.y;

		m_pInfluenceMap->updateActor(dt, p->position);

		p = m_poolEntity.next_nocheck(p);
	}
}

void EntityAspect::updatePlayerPositionCamera()
{
	m_playerController.updatePlayerHuman(m_pPlayer, *this);
}

void EntityAspect::updateActorTransforms()
{
	//std::vector<U16> indices;
	//indices.reserve(m_poolRender.size());

	//std::vector<vx::TransformGpu> transforms;
	//transforms.reserve(m_poolRender.size());

	I32 count = m_poolRender.size();

	auto marker = m_allocator.getMarker();
	SCOPE_EXIT
	{
		m_allocator.clear(marker);
	};

	//U16* pIndices = (U16*)m_allocator.allocate(sizeof(U16) * count, 4);
	//std::pair<U16, U16>* pIndices_Sorted = (std::pair<U16, U16>*)m_allocator.allocate(sizeof(std::pair<U16, U16>) * count, 4);
//	vx::TransformGpu* pTransforms = (vx::TransformGpu*)m_allocator.allocate(sizeof(vx::TransformGpu) * count, 16);
	auto pTransforms = (vx::TransformGpu*)_aligned_malloc(sizeof(vx::TransformGpu) * count, 16);
	auto indices = new U32[count];
	U32 index = 0;

	auto p = m_poolRender.first();
	while (p != nullptr)
	{
		auto &entity = m_poolEntity[p->entityIndex];
		//auto physics = m_poolPhysics[entity.physics];

		__m128 v = { entity.orientation.y, entity.orientation.x, 0, 0 };
		v = vx::QuaternionRotationRollPitchYawFromVector(v);
		auto packedRotation = GpuFunctions::packQRotation(v);

		vx::TransformGpu transform;
		transform.translation = entity.position;
		transform.scaling = 1.0f;
		transform.packedQRotation = packedRotation;

		//pIndices[index] = p->gpuIndex;
		//pIndices_Sorted[index] = std::make_pair(p->gpuIndex, index);
		indices[index] = p->gpuIndex;
		pTransforms[index] = transform;
		++index;

		p = m_poolRender.next_nocheck(p);
	}

	/*std::sort(pIndices_Sorted, pIndices_Sorted + count, [](const std::pair<U16, U16> &l, const std::pair<U16, U16> &r)
	{
		return l.first < r.first;
	});

	for (I32 i = 0; i < count; ++i)
	{
		U32 newIndex = i;
		U32 oldIndex = pIndices_Sorted[i].second;

		std::swap(pTransforms[newIndex], pTransforms[oldIndex]);
	}

	I32 offset = 0;
	U32 start = 0;
	while (count != 0)
	{
		U32 batchCount = 1;
		auto batchStart = pIndices_Sorted[offset].first;

		U32 expected = batchStart + 1;
		I32 remainingSize = count - offset;
		for (I32 i = 1; i < remainingSize; ++i)
		{
			auto nextIndex = pIndices_Sorted[offset + i].first;

			if (nextIndex != expected)
			{
				break;
			}

			++expected;
			++batchCount;
		}

		//m_renderAspect.updateTransforms(pTransforms + offset, batchStart, batchCount);

		count -= batchCount;
		offset += batchCount;
	}*/

	auto data = new RenderUpdateDataTransforms();
	data->transforms = pTransforms;
	data->indices = indices;
	data->count = count;

	RenderUpdateTask task;
	task.type = RenderUpdateTask::Type::UpdateDynamicTransforms;
	task.ptr = data;
	m_renderAspect.queueUpdateTask(task);
}

Component::Input& EntityAspect::getComponentInput(U16 i)
{
	return m_poolInput[i];
}

void EntityAspect::handleKeyboard(const vx::Keyboard &keyboard)
{
	m_playerController.handleKeyboard(m_pPlayer, keyboard, *this);
}

void EntityAspect::handleMouse(const vx::Mouse &mouse, const F32 dt)
{
	m_playerController.handleMouse(m_pPlayer, mouse, dt, *this);
}

void EntityAspect::keyPressed(U16 key)
{
	m_playerController.keyPressed(key, *this);
}

void EntityAspect::handleEvent(const Event &evt)
{
	switch (evt.type)
	{
	case EventType::Ingame_Event:
		handleIngameEvent(evt);
		break;
	case EventType::File_Event:
		handleFileEvent(evt);
		break;
	default:
		break;
	}
}

void EntityAspect::handleFileEvent(const Event &evt)
{
	if (evt.code == (U32)FileEvent::Scene_Loaded)
	{
		Event e;
		e.arg1 = evt.arg1;
		e.type = EventType::Ingame_Event;
		e.code = (U32)IngameEvent::Level_Started;

		auto pEvtManager = Locator::getEventManager();
		pEvtManager->addEvent(e);

		m_pCurrentScene = (const Scene*)evt.arg1.ptr;
	}
}

void EntityAspect::handleIngameEvent(const Event &evt)
{
	IngameEvent e = (IngameEvent)evt.code;
	switch (e)
	{
	case IngameEvent::Created_NavGraph:
	{
		m_pNavGraph = (const NavGraph*)evt.arg1.ptr;

		VX_ASSERT(m_pNavGraph->getNodeCount() <= s_maxNavNodes);
		//m_pCurrentScene = (const Scene*)evt.userData;

		auto spawns = m_pCurrentScene->getSpawns();
		auto spawnCount = m_pCurrentScene->getSpawnCount();

		for (U32 i = 0; i < spawnCount; ++i)
		{
			auto &it = spawns[i];

			if (it.type == PlayerType::Human)
			{
				createPlayerEntity(it.position);
			}
			else
			{
				auto evtManager = Locator::getEventManager();

			//	it.position, it.sid, 2.0f, gpuIndex

				auto &actors = m_pCurrentScene->getActors();
				auto itActor = actors.find(it.sid);

				CreateActorData* data = new CreateActorData();
				data->material = itActor->material;
				data->mesh = itActor->mesh;
				data->pScene = m_pCurrentScene;
				data->transform.m_rotation = vx::float3(0);
				data->transform.m_scaling = 1.0f;
				data->transform.m_translation = it.position;
				data->index = i;

				RenderUpdateTask task;
				task.ptr = data;
				task.type = RenderUpdateTask::Type::CreateActorGpuIndex;
				m_renderAspect.queueUpdateTask(task);

				/*Event e;
				e.type = EventType::Ingame_Event;
				e.code = (U32)IngameEvent::Create_Actor;
				e.arg1 = i;
				e.arg2 = data;

				evtManager->addEvent(e);*/
			}
		}

	}break;
	case IngameEvent::Created_InfluenceMap:
	{
		m_pInfluenceMap = (InfluenceMap*)evt.arg1.ptr;
	}break;
	case IngameEvent::Created_Actor_GPU:
	{
		auto spawns = m_pCurrentScene->getSpawns();
		auto spawnIndex = evt.arg1.u32;
		auto gpuIndex = evt.arg2.u32;

		auto &it = spawns[spawnIndex];
		createActorEntity(it.position, 2.0f, gpuIndex);
	}break;
	default:
		break;
	}
}