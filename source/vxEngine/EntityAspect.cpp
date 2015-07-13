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
#include "PhysicsAspect.h"
#include <vxEngineLib/Entity.h>
#include "ComponentActor.h"
#include "ComponentInput.h"
#include "PhysicsDefines.h"
#include <vxEngineLib/Scene.h>
#include <vxEngineLib/Actor.h>
#include <vxEngineLib/Event.h>
#include <vxEngineLib/EventTypes.h>
#include <vxEngineLib/Locator.h>
#include <vxEngineLib/EventManager.h>
#include <vxEngineLib/InfluenceMap.h>
#include <vxEngineLib/NavGraph.h>
#include <vxEngineLib/EventsIngame.h>
#include "EngineGlobals.h"
#include <vxEngineLib/FileEvents.h>
#include <vxEngineLib/MeshInstance.h>
#include <vxResourceAspect/FileAspect.h>
#include <vxEngineLib/Material.h>
#include "ComponentRender.h"
#include <vxEngineLib/RenderAspectInterface.h>
#include "TaskSceneCreateStaticMeshes.h"
#include "TaskManager.h"
#include "TaskSceneCreateActorsGpu.h"
#include <vxEngineLib/CreateActorData.h>
#include <vxEngineLib/Spawn.h>
#include "ComponentUsable.h"
#include "AllocationManager.h"
#include "ComponentPhysics.h"
#include "CreatedActorData.h"

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

bool EntityAspect::initialize(vx::StackAllocator* pAllocator, TaskManager* taskManager, AllocationManager* allocManager)
{
	m_coldData = vx::make_unique<ColdData>();

	m_componentUsableManager.initialize(g_maxEntities, pAllocator);
	m_componentRenderManager.initialize(g_maxEntities, pAllocator);
	m_componentPhysicsManager.initialize(g_maxEntities, pAllocator);
	m_componentInputManager.initialize(g_maxEntities, pAllocator);
	m_componentActorManager.initialize(g_maxEntities, pAllocator);
	EntityAspectCpp::createPool(g_maxEntities, 16, pAllocator, &m_poolEntity);

	//const auto pathChunkSize = s_maxNavNodes * sizeof(vx::float3);
	//const auto pathPoolSize = g_maxEntities * pathChunkSize;
	//m_poolAllocatorPath = vx::PoolAllocator(pAllocator->allocate(pathPoolSize, 8), pathPoolSize, pathChunkSize, __alignof(vx::float3));

	m_allocator = vx::StackAllocator(pAllocator->allocate(5 KBYTE, 16), 5 KBYTE);
	allocManager->registerAllocator(&m_allocator, "EntityAspect");

	m_playerController.initialize(pAllocator);

	m_taskManager = taskManager;

	return true;
}

void EntityAspect::shutdown()
{
	if (m_coldData)
	{
		m_coldData.reset();
	}
	m_poolEntity.release();
	m_componentActorManager.shutdown();
	m_componentInputManager.shutdown();
	m_componentPhysicsManager.shutdown();
	m_componentRenderManager.shutdown();
	m_componentUsableManager.shutdown();
}

void EntityAspect::builEntityQuadTree()
{
	auto size = m_componentInputManager.getSize();
	if (size == 0)
		return;

	std::vector<QuadTreeData> data;
	data.reserve(size);

	m_componentInputManager.getQuadTreeData(&data, &m_componentPhysicsManager, &m_poolEntity);

	m_quadTree.clear();
	m_quadTree.insert(data.data(), data.size());
}

void EntityAspect::createPlayerEntity(const vx::float3 &position)
{
	if (m_coldData->m_pPlayer == nullptr)
	{
		u16 entityIndex;
		m_coldData->m_pPlayer = m_poolEntity.createEntry(&entityIndex);
		m_poolEntity[entityIndex].position = position;

		u16 inputComponentIndex;
		auto componentInput = m_componentInputManager.createComponent(entityIndex, &inputComponentIndex);

		auto physicsAspect = Locator::getPhysicsAspect();
		auto controller = physicsAspect->createActor(position, g_heightStanding);
		u16 componentPhysicsIndex;
		auto componentPhysics = m_componentPhysicsManager.createComponent(position, controller, entityIndex, &componentPhysicsIndex);

		m_coldData->m_pPlayer->addComponent(*componentPhysics, componentPhysicsIndex);
		m_coldData->m_pPlayer->addComponent(*componentInput, inputComponentIndex);

		m_playerController.initializePlayer(componentInput, g_dt, m_coldData->m_pPlayer, Locator::getRenderAspect());
	}
}

void EntityAspect::createActorEntity(const CreateActorData &data)
{
	auto transform = data.getTransform();

	u16 entityIndex;
	auto pEntity = m_poolEntity.createEntry(&entityIndex);
	pEntity->position = transform.m_translation;

	u16 componentPhysicsIndex;
	auto componentPhysics = m_componentPhysicsManager.createComponent(transform.m_translation, data.getController(), entityIndex, &componentPhysicsIndex);

	u16 inputComponentIndex;
	auto componentInput = m_componentInputManager.createComponent(entityIndex, &inputComponentIndex);


	auto gpuIndex = data.getGpuIndex();

	//createComponentPhysics(data, entityIndex);

	u16 componentRenderIndex;
	auto componentRender = m_componentRenderManager.createComponent(transform, gpuIndex, entityIndex, &componentRenderIndex);

	u16 componentActorIndex;
	auto componentActor = m_componentActorManager.createComponent(entityIndex, pEntity, componentInput, componentPhysics, &m_quadTree, &componentActorIndex);

	pEntity->addComponent(*componentRender, componentRenderIndex);
	pEntity->addComponent(*componentPhysics, componentPhysicsIndex);
	pEntity->addComponent(*componentInput, inputComponentIndex);
	pEntity->addComponent(*componentActor, componentActorIndex);

	auto createdData = new CreatedActorData();
	createdData->entity = pEntity;
	createdData->componentActor = componentActor;
	createdData->componentPhysics = componentPhysics;

	vx::Event evt;
	evt.type = vx::EventType::Ingame_Event;
	evt.code = (u32)IngameEvent::Created_Actor;
	evt.arg1.ptr = createdData;

	auto pEvtManager = Locator::getEventManager();
	pEvtManager->addEvent(evt);
}

void EntityAspect::update(f32 dt, ActionManager* actionManager)
{
	m_componentActorManager.update(actionManager, &m_allocator);

	builEntityQuadTree();

	m_componentInputManager.update(dt, &m_componentPhysicsManager, &m_poolEntity);

	m_componentPhysicsManager.update(&m_poolEntity);

	m_playerController.update();

	m_componentRenderManager.update(&m_allocator, Locator::getRenderAspect(), m_poolEntity);
}

Component::Input& EntityAspect::getComponentInput(u16 i)
{
	return m_componentInputManager[i];
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

		auto scene = (const Scene*)evt.arg2.ptr;

		auto renderAspect = Locator::getRenderAspect();

		m_taskManager->queueTask<TaskSceneCreateActorsGpu>(scene, renderAspect);
		m_taskManager->queueTask<TaskSceneCreateStaticMeshes>(scene, renderAspect);

		auto spawns = scene->getSpawns();
		auto spawnCount = scene->getSpawnCount();
		auto evtManager = Locator::getEventManager();

		for (u32 i = 0; i < spawnCount; ++i)
		{
			auto &it = spawns[i];

			if (it.type == PlayerType::Human)
			{
				createPlayerEntity(it.position);
			}
		}

		auto bounds = scene->getNavMesh().getBounds();
		bounds.min.y -= 0.1f;
		bounds.max.y += 0.1f;
		m_quadTree.initialize(bounds, vx::uint2(5), 64);
		m_coldData->m_pCurrentScene = scene;
	}
}

void EntityAspect::createEntityUsable(const MeshInstance &instance, u32 gpuIndex)
{
	u16 entityIndex = 0;
	auto entity = m_poolEntity.createEntry(&entityIndex);

	u16 usableIndex = 0;
	auto componentUsable = m_componentUsableManager.createComponent(instance, entityIndex, &usableIndex);

	auto transform = instance.getTransform();

	u16 renderIndex = 0;
	auto componentRender = m_componentRenderManager.createComponent(transform, gpuIndex, entityIndex, &renderIndex);

	auto position = transform.m_translation;

	entity->position = position;
	entity->qRotation = transform.m_qRotation;
	entity->addComponent(*componentRender, renderIndex);
	entity->addComponent(*componentUsable, usableIndex);
}

void EntityAspect::handleIngameEvent(const vx::Event &evt)
{
	IngameEvent e = (IngameEvent)evt.code;
	switch (e)
	{
	case IngameEvent::Gpu_AddedActor:
	{
		CreateActorData* data = (CreateActorData*)evt.arg1.ptr;

		if (data->isValid())
		{
			createActorEntity(*data);
			delete(data);
		}
	}break;
	case IngameEvent::Physx_AddedActor:
	{
		CreateActorData* data = (CreateActorData*)evt.arg1.ptr;
		if (data->isValid())
		{
			createActorEntity(*data);
			delete(data);
		}

	}break;
	case IngameEvent::Gpu_AddedStaticEntity:
	{
		auto instance = (MeshInstance*)evt.arg1.ptr;
		auto gpuIndex = evt.arg2.u32;

		auto animSid = instance->getAnimationSid();
		if (animSid.value != 0)
		{
			createEntityUsable(*instance, gpuIndex);
		}
	}break;
	case IngameEvent::Created_NavGraph:
	{
	}break;
	default:
		break;
	}
}

void EntityAspect::onPressedActionKey()
{

}