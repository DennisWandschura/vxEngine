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
#include <vxEngineLib/Message.h>
#include <vxEngineLib/MessageTypes.h>
#include <vxEngineLib/Locator.h>
#include <vxEngineLib/MessageManager.h>
#include <vxEngineLib/InfluenceMap.h>
#include <vxEngineLib/NavGraph.h>
#include <vxEngineLib/IngameMessage.h>
#include "EngineGlobals.h"
#include <vxEngineLib/FileMessage.h>
#include <vxEngineLib/MeshInstance.h>
#include <vxResourceAspect/FileAspect.h>
#include <vxEngineLib/Material.h>
#include "ComponentRender.h"
#include <vxEngineLib/RenderAspectInterface.h>
#include "TaskSceneCreateStaticMeshes.h"
#include <vxEngineLib/TaskManager.h>
#include "TaskSceneCreateActorsGpu.h"
#include <vxEngineLib/CreateActorData.h>
#include <vxEngineLib/Spawn.h>
#include "ComponentUsable.h"
#include <vxLib/Allocator/AllocationProfiler.h>
#include "ComponentPhysics.h"
#include "CreatedActorData.h"
#include <vxEngineLib/CreateDynamicMeshData.h>
#include "TaskPhysxCreateJoints.h"

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

bool EntityAspect::initialize(vx::StackAllocator* pAllocator, vx::TaskManager* taskManager, vx::AllocationProfiler* allocManager)
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

#if _VX_MEM_PROFILE
	allocManager->registerAllocator(&m_allocator, "EntityAspect");
#endif

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
		auto componentPhysics = m_componentPhysicsManager.createComponent(position, controller, nullptr, entityIndex, &componentPhysicsIndex);

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
	auto componentPhysics = m_componentPhysicsManager.createComponent(transform.m_translation, data.getController(), nullptr, entityIndex, &componentPhysicsIndex);

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

	vx::Message evt;
	evt.type = vx::MessageType::Ingame_Event;
	evt.code = (u32)IngameMessage::Created_Actor;
	evt.arg1.ptr = createdData;

	auto pEvtManager = Locator::getMessageManager();
	pEvtManager->addMessage(evt);
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

void EntityAspect::handleMessage(const vx::Message &evt)
{
	switch (evt.type)
	{
	case vx::MessageType::Ingame_Event:
		handleIngameMessage(evt);
		break;
	case vx::MessageType::File_Event:
		handleFileEvent(evt);
		break;
	default:
		break;
	}
}

void EntityAspect::handleFileEvent(const vx::Message &evt)
{
	if (evt.code == (u32)vx::FileMessage::Scene_Loaded)
	{
		vx::Message e;
		e.arg1 = evt.arg2;
		e.type = vx::MessageType::Ingame_Event;
		e.code = (u32)IngameMessage::Level_Started;

		auto pEvtManager = Locator::getMessageManager();
		pEvtManager->addMessage(e);

		auto scene = (const Scene*)evt.arg2.ptr;

		auto renderAspect = Locator::getRenderAspect();

		m_taskManager->pushTask(new TaskSceneCreateActorsGpu(scene, renderAspect), false);
		m_taskManager->pushTask(new TaskSceneCreateStaticMeshes(scene, renderAspect), false);

		auto spawns = scene->getSpawns();
		auto spawnCount = scene->getSpawnCount();
		auto evtManager = Locator::getMessageManager();

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

void EntityAspect::createDynamicMesh(const CreateDynamicMeshData &data)
{
	u16 entityIndex = 0;
	auto entity = m_poolEntity.createEntry(&entityIndex);

	auto transform = data.m_meshInstance->getTransform();

	//u16 usableIndex = 0;
	//auto componentUsable = m_componentUsableManager.createComponent(data.m_meshInstance, entityIndex, &usableIndex);

	u16 renderIndex = 0;
	auto componentRender = m_componentRenderManager.createComponent(transform, data.m_gpuIndex, entityIndex, &renderIndex);

	u16 physicsIndex = 0;
	auto componentPhysics = m_componentPhysicsManager.createComponent(transform.m_translation, nullptr, data.m_rigidDynamic, entityIndex, &physicsIndex);

	entity->position = transform.m_translation;
	entity->qRotation = transform.m_qRotation;
	entity->addComponent(*componentRender, renderIndex);
	entity->addComponent(*componentPhysics, physicsIndex);
	//entity->addComponent(*componentUsable, usableIndex);
}

void EntityAspect::handleIngameMessage(const vx::Message &evt)
{
	IngameMessage e = (IngameMessage)evt.code;
	switch (e)
	{
	case IngameMessage::Physx_AddedDynamicMesh:
	{
		CreateDynamicMeshData* data = (CreateDynamicMeshData*)evt.arg1.ptr;
		data->decrement();

		if (data->m_flags == 0 && data->isValid())
		{
			createDynamicMesh(*data);
			delete(data);
		}

	}break;
	case IngameMessage::Gpu_AddedActor:
	{
		CreateActorData* data = (CreateActorData*)evt.arg1.ptr;

		if (data->isValid())
		{
			createActorEntity(*data);
			delete(data);
		}
	}break;
	case IngameMessage::Physx_AddedActor:
	{
		CreateActorData* data = (CreateActorData*)evt.arg1.ptr;
		if (data->isValid())
		{
			createActorEntity(*data);
			delete(data);
		}

	}break;
	case IngameMessage::Gpu_AddedDynamicMesh:
	{
		CreateDynamicMeshData* data = (CreateDynamicMeshData*)evt.arg1.ptr;
		data->decrement();

		if (data->m_flags == 0 && data->isValid())
		{
			createDynamicMesh(*data);
			delete(data);
		}
	}break;
	case IngameMessage::Physx_CreatedScene:
	{
		auto physicsAspect = Locator::getPhysicsAspect();
		auto scene = (Scene*)evt.arg1.ptr;
		auto fetchEvt = physicsAspect->getEventPhysicsFetch();

		std::vector<shared_ptr<Event>> events;
		events.push_back(fetchEvt);
		auto task = new TaskPhysxCreateJoints(scene, physicsAspect, std::move(events));

		m_taskManager->pushTask(task, false);
	}break;
	case IngameMessage::Created_NavGraph:
	{
	}break;
	default:
		break;
	}
}

void EntityAspect::onPressedActionKey()
{

}