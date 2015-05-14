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
#include "EntityAspect.h"
#include <characterkinematic/PxController.h>
#include "PhysicsAspect.h"
#include "Entity.h"
#include "ComponentActor.h"
#include "ComponentInput.h"
#include "PhysicsDefines.h"
#include "Scene.h"
#include "Transform.h"
#include "RenderAspect.h"
#include "ComponentRender.h"
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
#include "State.h"
#include "EngineGlobals.h"
#include "Transition.h"

#include "ConditionActorFollowingPath.h"
#include "ConditionActorHasPath.h"
#include "ActionFollowPath.h"
#include "ActionSetFollowPath.h"

namespace
{
	template<typename T>
	void createPool(u16 capacity, u8 alignment, vx::StackAllocator* pAllocator, Pool<T> *pPool)
	{
		pPool->initialize(pAllocator->allocate(sizeof(T) * capacity, alignment), capacity);
	}
}

EntityAspect::EntityAspect(PhysicsAspect &physicsAspect, RenderAspect &renderAspect)
	:m_playerController(),
	m_physicsAspect(physicsAspect),
	m_renderAspect(renderAspect),
	m_coldData()
{
}

bool EntityAspect::initialize(vx::StackAllocator* pAllocator)
{
	m_coldData = vx::make_unique<ColdData>();

	createPool(g_maxEntities, 16, pAllocator, &m_poolInput);
	createPool(g_maxEntities, 16, pAllocator, &m_poolRender);
	createPool(g_maxEntities, 16, pAllocator, &m_poolEntity);
	createPool(g_maxEntities, 16, pAllocator, &m_coldData->m_poolActor);

	//const auto pathChunkSize = s_maxNavNodes * sizeof(vx::float3);
	//const auto pathPoolSize = g_maxEntities * pathChunkSize;
	//m_poolAllocatorPath = vx::PoolAllocator(pAllocator->allocate(pathPoolSize, 8), pathPoolSize, pathChunkSize, __alignof(vx::float3));

	m_allocator = vx::StackAllocator(pAllocator->allocate(1 KBYTE, 16), 1 KBYTE);

	m_playerController.initialize(pAllocator);

	return true;
}

void EntityAspect::shutdown()
{
	if (m_coldData)
	{
		m_coldData->m_poolActor.release();
		m_coldData.reset();
	}
	m_poolEntity.release();
	m_poolRender.release();
	m_poolInput.release();
}

void EntityAspect::createComponentPhysics(const vx::float3 &position, u16 entityIndex, f32 height)
{
	auto pActor = m_physicsAspect.createActor(position, height);

	m_poolEntity[entityIndex].position = position;
	m_poolEntity[entityIndex].pRigidActor = pActor;
}

Component::Actor* EntityAspect::createComponentActor(u16 entityIndex, EntityActor* entity, Component::Input* componentInput, u16* actorIndex)
{
	auto pActor = m_coldData->m_poolActor.createEntry(actorIndex);
	pActor->entityIndex = entityIndex;

	pActor->m_data = vx::make_unique<Component::ActorData>();
	pActor->m_busy = 0;
	pActor->m_followingPath = 0;

	ActionFollowPath* actionFollowPath = new ActionFollowPath(entity, componentInput, pActor->m_data.get());
	ActionSetFollowPath* actionSetFollowPath = new ActionSetFollowPath(actionFollowPath, pActor->m_data.get());

	State* waitingState = new State();
	State* movingState = new State();

	ConditionActorHasPath* conditionActorHasPath = new ConditionActorHasPath(pActor->m_data.get());
	ConditionActorNotFollowingPath* conditionActorNotFollowingPath = new ConditionActorNotFollowingPath(pActor);

	Transition* transitionWaitingToMoving = new Transition(conditionActorHasPath, movingState);
	waitingState->addTransition(transitionWaitingToMoving);
	transitionWaitingToMoving->addAction(actionSetFollowPath);
	transitionWaitingToMoving->addAction(actionFollowPath);

	Transition* transitionMovingToWaiting = new Transition(conditionActorNotFollowingPath, waitingState);
	movingState->addTransition(transitionMovingToWaiting);

	pActor->m_stateMachine.addState(waitingState);
	pActor->m_stateMachine.addState(movingState);

	pActor->m_stateMachine.setInitialState(waitingState);

	return pActor;
}

void EntityAspect::createPlayerEntity(const vx::float3 &position)
{
	if (m_coldData->m_pPlayer == nullptr)
	{
		u16 entityIndex;
		m_coldData->m_pPlayer = m_poolEntity.createEntry(&entityIndex);

		auto pInput = m_poolInput.createEntry(&m_coldData->m_pPlayer->input);
		pInput->entityIndex = entityIndex;
		pInput->orientation.x = 0.0f;

		createComponentPhysics(position, entityIndex, g_heightStanding);

		m_playerController.initializePlayer(pInput, g_dt, m_coldData->m_pPlayer, &m_renderAspect);
	}
}

void EntityAspect::createActorEntity(const vx::float3 &position, f32 height, u32 gpuIndex)
{
	u16 entityIndex;
	auto pEntity = m_poolEntity.createEntry(&entityIndex);
	auto pInput = m_poolInput.createEntry(&pEntity->input);
	pInput->entityIndex = entityIndex;

	createComponentPhysics(position, entityIndex, height);

	auto pRender = m_poolRender.createEntry(&pEntity->render);
	pRender->entityIndex = entityIndex;
	pRender->gpuIndex = gpuIndex;

	auto pActor = createComponentActor(entityIndex, pEntity, pInput, &pEntity->actor);

	Event evt;
	evt.type = EventType::Ingame_Event;
	evt.code = (u32)IngameEvent::Created_Actor;
	evt.arg1.ptr = pEntity;
	evt.arg2.ptr = pActor;

	auto pEvtManager = Locator::getEventManager();
	pEvtManager->addEvent(evt);
}

void EntityAspect::updateInput(f32 dt)
{
	const vx::ivec4 velocityMask = { (s32)0xffffffff, 0, (s32)0xffffffff, 0};

	const __m128 vGravity = { 0, g_gravity * dt, 0, 0 };
	auto p = m_poolInput.first();
	while (p != nullptr)
	{
		auto &entity = m_poolEntity[p->entityIndex];

		entity.orientation = p->orientation;

		//__m128 vVelocity = { p->velocity.x, 0, p->velocity.z, 0.0f };
		__m128 vVelocity = vx::loadFloat(p->velocity);
		p->velocity.x = 0.0f;
		p->velocity.y = 0.0f;
		p->velocity.z = 0.0f;

		vVelocity = _mm_and_ps(vVelocity, velocityMask);
		vVelocity = _mm_add_ps(vVelocity, vGravity);

		m_physicsAspect.move(vVelocity, dt, entity.pRigidActor);

		p = m_poolInput.next_nocheck(p);
	}
}

void EntityAspect::updatePhysics_linear(f32 dt)
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

		p = m_poolEntity.next_nocheck(p);
	}
}

void EntityAspect::updatePlayerPositionCamera()
{
	m_playerController.update();
}

void EntityAspect::updateActorTransforms()
{
	s32 count = m_poolRender.size();

	auto marker = m_allocator.getMarker();
	SCOPE_EXIT
	{
		m_allocator.clear(marker);
	};


	auto totalSizeInBytes = sizeof(RenderUpdateDataTransforms) + (sizeof(vx::TransformGpu) + sizeof(u32)) * count;
	auto dataPtr = m_allocator.allocate(totalSizeInBytes, 16);

	vx::TransformGpu* pTransforms = (vx::TransformGpu*)(dataPtr + sizeof(RenderUpdateDataTransforms));
	u32* indices = (u32*)(pTransforms + count);
	u32 index = 0;

	auto p = m_poolRender.first();
	while (p != nullptr)
	{
		auto &entity = m_poolEntity[p->entityIndex];

		__m128 v = { entity.orientation.y, entity.orientation.x, 0, 0 };
		v = vx::quaternionRotationRollPitchYawFromVector(v);
		auto packedRotation = GpuFunctions::packQRotation(v);

		vx::TransformGpu transform;
		transform.translation = entity.position;
		transform.scaling = 1.0f;
		transform.packedQRotation = packedRotation;

		indices[index] = p->gpuIndex;
		pTransforms[index] = transform;
		++index;

		p = m_poolRender.next_nocheck(p);
	}

	RenderUpdateDataTransforms* data = (RenderUpdateDataTransforms*)dataPtr;
	data->count = count;

	RenderUpdateTask task;
	task.type = RenderUpdateTask::Type::UpdateDynamicTransforms;
	m_renderAspect.queueUpdateTask(task, dataPtr, totalSizeInBytes);
}

Component::Input& EntityAspect::getComponentInput(u16 i)
{
	return m_poolInput[i];
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
	if (evt.code == (u32)FileEvent::Scene_Loaded)
	{
		Event e;
		e.arg1 = evt.arg1;
		e.type = EventType::Ingame_Event;
		e.code = (u32)IngameEvent::Level_Started;

		auto pEvtManager = Locator::getEventManager();
		pEvtManager->addEvent(e);

		m_coldData->m_pCurrentScene = (const Scene*)evt.arg1.ptr;
	}
}

void EntityAspect::handleIngameEvent(const Event &evt)
{
	IngameEvent e = (IngameEvent)evt.code;
	switch (e)
	{
	case IngameEvent::Created_NavGraph:
	{
		//m_pNavGraph = (const NavGraph*)evt.arg1.ptr;

		//VX_ASSERT(m_pNavGraph->getNodeCount() <= s_maxNavNodes);
		//m_pCurrentScene = (const Scene*)evt.userData;

		auto spawns = m_coldData->m_pCurrentScene->getSpawns();
		auto spawnCount = m_coldData->m_pCurrentScene->getSpawnCount();

		for (u32 i = 0; i < spawnCount; ++i)
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

				auto &actors = m_coldData->m_pCurrentScene->getActors();
				auto itActor = actors.find(it.sid);

				CreateActorData data;
				data.material = itActor->m_material;
				data.mesh = itActor->m_mesh;
				data.pScene = m_coldData->m_pCurrentScene;
				data.transform.m_rotation = vx::float3(0);
				data.transform.m_scaling = 1.0f;
				data.transform.m_translation = it.position;
				data.index = i;

				RenderUpdateTask task;
				task.type = RenderUpdateTask::Type::CreateActorGpuIndex;
				m_renderAspect.queueUpdateTask(task, (u8*)&data, sizeof(CreateActorData));

				/*Event e;
				e.type = EventType::Ingame_Event;
				e.code = (u32)IngameEvent::Create_Actor;
				e.arg1 = i;
				e.arg2 = data;

				evtManager->addEvent(e);*/
			}
		}

	}break;
	case IngameEvent::Created_InfluenceMap:
	{
		//m_pInfluenceMap = (InfluenceMap*)evt.arg1.ptr;
	}break;
	case IngameEvent::Created_Actor_GPU:
	{
		auto spawns = m_coldData->m_pCurrentScene->getSpawns();
		auto spawnIndex = evt.arg1.u32;
		auto gpuIndex = evt.arg2.u32;

		auto &it = spawns[spawnIndex];
		createActorEntity(it.position, 2.0f, gpuIndex);
	}break;
	default:
		break;
	}
}