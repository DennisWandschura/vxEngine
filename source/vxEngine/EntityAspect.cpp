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
#include <vxEngineLib/Scene.h>
#include <vxEngineLib/Transform.h>
#include <vxRenderAspect/RenderAspect.h>
#include "ComponentRender.h"
#include <vxEngineLib/Actor.h>
#include <vxEngineLib/Event.h>
#include <vxEngineLib/EventTypes.h>
#include <vxEngineLib/Locator.h>
#include <vxEngineLib/EventManager.h>
#include "InfluenceMap.h"
#include "NavGraph.h"
#include <vxEngineLib/EventsIngame.h>
#include <vxLib/ScopeGuard.h>
#include <vxEngineLib/Spawn.h>
#include <vxRenderAspect/GpuFunctions.h>
#include <vxEngineLib/CreateActorData.h>
#include "State.h"
#include "EngineGlobals.h"
#include "Transition.h"
#include <vxEngineLib/FileEvents.h>

#include "ConditionActorFollowingPath.h"
#include "ConditionActorHasPath.h"
#include "ActionFollowPath.h"
#include "ActionSetFollowPath.h"
#include "ActionPrintText.h"
#include "ActionActorCreatePath.h"

namespace EntityAspectCpp
{
	template<typename T>
	void createPool(u16 capacity, u8 alignment, vx::StackAllocator* pAllocator, vx::Pool<T> *pPool)
	{
		pPool->initialize(pAllocator->allocate(sizeof(T) * capacity, alignment), capacity);
	}
}

EntityAspect::EntityAspect()
	:m_playerController(),
	m_coldData()
{
}

bool EntityAspect::initialize(vx::StackAllocator* pAllocator)
{
	m_coldData = vx::make_unique<ColdData>();

	EntityAspectCpp::createPool(g_maxEntities, 16, pAllocator, &m_poolInput);
	EntityAspectCpp::createPool(g_maxEntities, 16, pAllocator, &m_poolRender);
	EntityAspectCpp::createPool(g_maxEntities, 16, pAllocator, &m_poolEntity);
	EntityAspectCpp::createPool(g_maxEntities, 16, pAllocator, &m_coldData->m_poolActor);

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

void EntityAspect::createComponentPhysics(const CreateActorData &data, u16 entityIndex)
{

	m_poolEntity[entityIndex].position = data.getTransform().m_translation;
	m_poolEntity[entityIndex].pRigidActor = data.getController();
}

Component::Actor* EntityAspect::createComponentActor(u16 entityIndex, EntityActor* entity, Component::Input* componentInput, u16* actorIndex)
{
	auto pActor = m_coldData->m_poolActor.createEntry(actorIndex);
	pActor->entityIndex = entityIndex;

	pActor->m_data = vx::make_unique<Component::ActorData>();
	pActor->m_busy = 0;
	pActor->m_followingPath = 0;

	ActionFollowPath* actionFollowPath = new ActionFollowPath(entity, componentInput, pActor);
	ActionSetFollowPath* actionSetFollowPath = new ActionSetFollowPath(actionFollowPath, pActor->m_data.get());

	ActionActorCreatePath* actionActorCreatePath = new ActionActorCreatePath(pActor);

	State* waitingState = new State();
	State* movingState = new State();

	waitingState->addAction(actionActorCreatePath);

	ConditionActorHasPath* conditionActorHasPath = new ConditionActorHasPath(pActor->m_data.get());
	ConditionActorNotFollowingPath* conditionActorNotFollowingPath = new ConditionActorNotFollowingPath(pActor);

	Transition* transitionWaitingToMoving = new Transition(conditionActorHasPath, movingState);
	waitingState->addTransition(transitionWaitingToMoving);
	transitionWaitingToMoving->addAction(actionSetFollowPath);
	transitionWaitingToMoving->addAction(actionFollowPath);

	Transition* transitionMovingToWaiting = new Transition(conditionActorNotFollowingPath, waitingState);
	movingState->addTransition(transitionMovingToWaiting);

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

		auto physicsAspect = Locator::getPhysicsAspect();
		m_poolEntity[entityIndex].position = position;
		m_poolEntity[entityIndex].pRigidActor = physicsAspect->createActor(position, g_heightStanding);

		m_playerController.initializePlayer(pInput, g_dt, m_coldData->m_pPlayer, Locator::getRenderAspect());
	}
}

void EntityAspect::createActorEntity(const CreateActorData &data)
{
	u16 entityIndex;
	auto pEntity = m_poolEntity.createEntry(&entityIndex);
	auto pInput = m_poolInput.createEntry(&pEntity->input);
	pInput->entityIndex = entityIndex;

	auto transform = data.getTransform();
	auto height = data.getHeight();
	auto gpuIndex = data.getGpuIndex();

	createComponentPhysics(data, entityIndex);
	//createComponentPhysics(mesh, position, entityIndex);

	auto pRender = m_poolRender.createEntry(&pEntity->render);
	pRender->entityIndex = entityIndex;
	pRender->gpuIndex = gpuIndex;

	auto pActor = createComponentActor(entityIndex, pEntity, pInput, &pEntity->actor);

	vx::Event evt;
	evt.type = vx::EventType::Ingame_Event;
	evt.code = (u32)IngameEvent::Created_Actor;
	evt.arg1.ptr = pEntity;
	evt.arg2.ptr = pActor;

	auto pEvtManager = Locator::getEventManager();
	pEvtManager->addEvent(evt);
}

void EntityAspect::updateInput(f32 dt)
{
	const vx::ivec4 velocityMask = { (s32)0xffffffff, 0, (s32)0xffffffff, 0};
	auto physicsAspect = Locator::getPhysicsAspect();

	const __m128 vGravity = { 0, g_gravity * dt, 0, 0 };
	auto p = m_poolInput.first();
	while (p != nullptr)
	{
		auto &entity = m_poolEntity[p->entityIndex];

		entity.orientation = p->orientation;

		//__m128 vVelocity = { p->velocity.x, 0, p->velocity.z, 0.0f };
		__m128 vVelocity = vx::loadFloat4(p->velocity);
		p->velocity.x = 0.0f;
		p->velocity.y = 0.0f;
		p->velocity.z = 0.0f;

		vVelocity = _mm_and_ps(vVelocity, velocityMask);
		vVelocity = _mm_add_ps(vVelocity, vGravity);

		physicsAspect->move(vVelocity, dt, entity.pRigidActor);

		p = m_poolInput.next_nocheck(p);
	}
}

void EntityAspect::updatePhysics_linear(f32 dt)
{
	UNREFERENCED_PARAMETER(dt);

	auto p = m_poolEntity.first();
	while (p != nullptr)
	{
		auto contactOffset = p->pRigidActor->getContactOffset();
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

	auto totalSizeInBytes = sizeof(RenderUpdateDataTransforms) + sizeof(vx::TransformGpu) * count + sizeof(u32) * count;
	auto dataPtr = m_allocator.allocate(totalSizeInBytes, 16);

	vx::TransformGpu* pTransforms = (vx::TransformGpu*)(dataPtr + sizeof(RenderUpdateDataTransforms));
	u32* indices = (u32*)(dataPtr + sizeof(RenderUpdateDataTransforms) + sizeof(vx::TransformGpu) * count);
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

	RenderUpdateDataTransforms* renderUpdateData = (RenderUpdateDataTransforms*)dataPtr;
	renderUpdateData->count = count;

	RenderUpdateTask task;
	task.type = RenderUpdateTask::Type::UpdateDynamicTransforms;

	auto renderAspect = Locator::getRenderAspect();
	renderAspect->queueUpdateTask(task, dataPtr, totalSizeInBytes);
}

Component::Input& EntityAspect::getComponentInput(u16 i)
{
	return m_poolInput[i];
}

void EntityAspect::handleEvent(const vx::Event &evt)
{
	switch (evt.type)
	{
	case vx::EventType::Ingame_Event:
		handleIngameEvent(evt);
		break;
	case vx::EventType::File_Event:
		handleFileEvent(evt);
		break;
	default:
		break;
	}
}

void EntityAspect::handleFileEvent(const vx::Event &evt)
{
	if (evt.code == (u32)vx::FileEvent::Scene_Loaded)
	{
		vx::Event e;
		e.arg1 = evt.arg2;
		e.type = vx::EventType::Ingame_Event;
		e.code = (u32)IngameEvent::Level_Started;

		auto pEvtManager = Locator::getEventManager();
		pEvtManager->addEvent(e);

		m_coldData->m_pCurrentScene = (const Scene*)evt.arg2.ptr;
	}
}

void EntityAspect::handleIngameEvent(const vx::Event &evt)
{
	IngameEvent e = (IngameEvent)evt.code;
	switch (e)
	{
	case IngameEvent::Created_NavGraph:
	{
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
				auto &actors = m_coldData->m_pCurrentScene->getActors();
				auto itActor = actors.find(it.sid);

				vx::Transform transform;
				transform.m_qRotation = vx::float4(0, 0, 0, 1);
				transform.m_scaling = 1.0f;
				transform.m_translation = it.position;

				CreateActorData* data = new CreateActorData(transform, itActor->m_mesh, itActor->m_material,2.0f, i);

				RenderUpdateTask task;
				task.type = RenderUpdateTask::Type::CreateActorGpuIndex;

				std::size_t address = (std::size_t)data;

				auto renderAspect = Locator::getRenderAspect();
				renderAspect->queueUpdateTask(task, (u8*)&address, sizeof(std::size_t));

				vx::Event evt;
				evt.type = vx::EventType::Ingame_Event;
				evt.code = (u32)IngameEvent::Create_Actor_Physx;
				evt.arg1.ptr = data;
				
				auto evtManager = Locator::getEventManager();
				evtManager->addEvent(evt);
			}
		}

	}break;
	case IngameEvent::Created_Actor_GPU:
	{
		CreateActorData* data = (CreateActorData*)evt.arg1.ptr;

		if (data->isValid())
		{
			createActorEntity(*data);
			delete(data);
		}
	}break;
	case IngameEvent::Created_Actor_Physx:
	{
		CreateActorData* data = (CreateActorData*)evt.arg1.ptr;
		if (data->isValid())
		{
			createActorEntity(*data);
			delete(data);
		}

	}break;
	default:
		break;
	}
}