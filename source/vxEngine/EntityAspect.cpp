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
#include "ComponentActor.h"
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
#include <vxEngineLib/Material.h>
#include <vxEngineLib/Graphics/RenderAspectInterface.h>
#include "TaskSceneCreateStaticMeshes.h"
#include <vxEngineLib/TaskManager.h>
#include "TaskSceneCreateActors.h"
#include <vxEngineLib/CreateActorData.h>
#include <vxEngineLib/Spawn.h>
#include "ComponentAction.h"
#include <vxLib/Allocator/AllocationProfiler.h>
#include "CreatedActorData.h"
#include <vxEngineLib/CreateDynamicMeshData.h>
#include "TaskPhysxCreateJoints.h"
#include <vxLib/ScopeGuard.h>
#include <characterkinematic/PxController.h>

#include "ActionGrabEntity.h"

namespace EntityAspectCpp
{
	template<typename T>
	void createPool(u16 capacity, u8 alignment, vx::StackAllocator* pAllocator, vx::Pool<T> *pPool)
	{
		pPool->initialize(pAllocator->allocate(sizeof(T) * capacity, alignment), capacity);
	}

	const u32 g_maxEntities = 128;
}

struct EntityAspect::ColdData
{
	const Scene* m_pCurrentScene{ nullptr };
};

EntityAspect::EntityAspect()
	:m_playerController(),
	m_coldData()
{
}

EntityAspect::~EntityAspect()
{

}

bool EntityAspect::initialize(vx::StackAllocator* pAllocator, vx::TaskManager* taskManager, vx::AllocationProfiler* allocManager)
{
	m_coldData = vx::make_unique<ColdData>();

	m_componentActionManager.initialize(EntityAspectCpp::g_maxEntities, pAllocator);
	m_componentActorManager.initialize(EntityAspectCpp::g_maxEntities, pAllocator);
	EntityAspectCpp::createPool(EntityAspectCpp::g_maxEntities, 16, pAllocator, &m_poolEntityActor);
	EntityAspectCpp::createPool(EntityAspectCpp::g_maxEntities, 16, pAllocator, &m_poolEntityDynamic);

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
	m_poolEntityDynamic.release();
	m_poolEntityActor.release();
	m_componentActorManager.shutdown();
	m_componentActionManager.shutdown();
}

void EntityAspect::builEntityQuadTree()
{
	/*auto size = m_componentInputManager.getSize();
	if (size == 0)
		return;

	std::vector<QuadTreeData> data;
	data.reserve(size);

	m_componentInputManager.getQuadTreeData(&data, &m_componentPhysicsManager, &m_poolEntity);

	m_quadTree.clear();
	m_quadTree.insert(data.data(), data.size());*/
}

void EntityAspect::createPlayerEntity(const CreateActorData &data)
{
	if (m_entityHuman == nullptr)
	{
		auto transform = data.getTransform();
		auto controller = data.getController();

		m_entityHuman = &m_entityHumanData;
		m_entityHuman->m_position = transform.m_translation;
		m_entityHuman->m_controller = controller;
		m_entityHuman->m_orientation.x = 0;
		m_entityHuman->m_orientation.y = 0;
		m_entityHuman->m_footPositionY = 0;

		m_playerController.initializePlayer(g_dt, m_entityHuman, Locator::getRenderAspect(), &m_componentActionManager, Locator::getMessageManager());

		Locator::getPhysicsAspect()->setHumanActor(controller->getActor());
	}
}

void EntityAspect::createActorEntity(const CreateActorData &data)
{
	auto transform = data.getTransform();

	u16 entityIndex;
	auto entity = m_poolEntityActor.createEntry(&entityIndex);
	entity->m_position = transform.m_translation;
	entity->m_controller = data.getController();
	entity->m_orientation.x = 0;
	entity->m_orientation.y = 0;
	entity->m_gpuIndex = data.getGpuIndex();

	u16 componentActorIndex;
	auto componentActor = m_componentActorManager.createComponent(entityIndex, entity, &m_quadTree, &componentActorIndex);

	auto createdData = new CreatedActorData();
	createdData->entity = entity;
	createdData->componentActor = componentActor;

	vx::Message evt;
	evt.type = vx::MessageType::Ingame;
	evt.code = (u32)IngameMessage::Created_Actor;
	evt.arg1.ptr = createdData;

	auto pEvtManager = Locator::getMessageManager();
	pEvtManager->addMessage(evt);
}

void EntityAspect::updateEntityActor(f32 dt)
{
	auto count = m_poolEntityActor.size();
	if (count != 0)
	{
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

		auto physicsAspect = Locator::getPhysicsAspect();
		auto ptr = m_poolEntityActor.first();
		while (ptr != nullptr)
		{
			ptr->update(dt, physicsAspect, pTransforms, indices, index);

			++index;
			ptr = m_poolEntityActor.next_nocheck(ptr);
		}

		RenderUpdateDataTransforms* renderUpdateData = (RenderUpdateDataTransforms*)dataPtr;
		renderUpdateData->count = count;

		Locator::getRenderAspect()->queueUpdate(RenderUpdateTaskType::UpdateDynamicTransforms, dataPtr, totalSizeInBytes);
	}
}

void EntityAspect::updateEntityDynamic(f32 dt)
{
	auto count = m_poolEntityDynamic.size();
	if (count != 0)
	{
		auto marker = m_allocator.getMarker();
		SCOPE_EXIT
		{
			m_allocator.clear(marker);
		};

		auto renderUpdateData = (RenderUpdateDataTransforms*)m_allocator.allocate(sizeof(RenderUpdateDataTransforms), 8);

		vx::TransformGpu* pTransforms = (vx::TransformGpu*)m_allocator.allocate(sizeof(vx::TransformGpu) * count, 16);
		u32* indices = (u32*)m_allocator.allocate(sizeof(u32) * count, 4);

		u8* last = (u8*)(indices + count);
		auto dataSize = last - reinterpret_cast<u8*>(renderUpdateData);

		u32 index = 0;
		auto ptr = m_poolEntityDynamic.first();
		while (ptr != nullptr)
		{
			ptr->update(dt, pTransforms, indices, &index);

			ptr = m_poolEntityDynamic.next_nocheck(ptr);
		}
		count = index;

		if (count != 0)
		{
			renderUpdateData->transforms = pTransforms;
			renderUpdateData->indices = indices;
			renderUpdateData->count = count;

			Locator::getRenderAspect()->queueUpdate(RenderUpdateTaskType::UpdateDynamicTransforms, (u8*)renderUpdateData, dataSize);
		}
	}
}

void EntityAspect::update(f32 dt, ActionManager* actionManager)
{
	if (m_entityHuman)
	{
		m_entityHuman->update(dt);
	}

	updateEntityActor(dt);
	updateEntityDynamic(dt);
	m_componentActionManager.update();

	m_playerController.update();
}

void EntityAspect::handleMessage(const vx::Message &evt)
{
	switch (evt.type)
	{
	case vx::MessageType::Ingame:
		handleIngameMessage(evt);
		break;
	case vx::MessageType::File:
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
		e.type = vx::MessageType::Ingame;
		e.code = (u32)IngameMessage::Level_Started;

		auto pEvtManager = Locator::getMessageManager();
		pEvtManager->addMessage(e);

		auto scene = (const Scene*)evt.arg2.ptr;

		auto renderAspect = Locator::getRenderAspect();
		auto physicsAspect = Locator::getPhysicsAspect();

		auto fetchEvt = physicsAspect->getEventPhysicsFetch();
		auto evtBlock = Event::createEvent();
		physicsAspect->addBlockEvent(evtBlock);

		std::vector<Event> events;
		events.push_back(fetchEvt);

		m_taskManager->pushTask(new TaskSceneCreateActors(evtBlock, std::move(events),scene, renderAspect, physicsAspect));
		m_taskManager->pushTask(new TaskSceneCreateStaticMeshes(scene, renderAspect));


		auto bounds = scene->getNavMesh().getBounds();
		bounds.min.y -= 0.1f;
		bounds.max.y += 0.1f;
		m_quadTree.initialize(bounds, vx::uint2(5), 64);
		m_coldData->m_pCurrentScene = scene;
	}
}

void EntityAspect::createDynamicMesh(const CreateDynamicMeshData &data)
{
	auto transform = data.getMeshInstance()->getTransform();
	auto rigidDynamic = data.getRigidDynamic();

	u16 index;
	auto entity = m_poolEntityDynamic.createEntry(&index);
	entity->m_position = transform.m_translation;
	entity->m_qRotation = transform.m_qRotation;
	entity->m_rigidDynamic = rigidDynamic;
	entity->m_gpuIndex = data.getGpuIndex();

	auto action = new ActionGrabEntity(m_entityHuman, entity);

	auto componentAction = m_componentActionManager.createComponent(rigidDynamic, action, index, &entity->m_actionComponentIndex);
	VX_ASSERT(componentAction);
}

void EntityAspect::handleIngameMessage(const vx::Message &evt)
{
	IngameMessage e = (IngameMessage)evt.code;
	switch (e)
	{
	case IngameMessage::Physx_AddedDynamicMesh:
	{
		CreateDynamicMeshData* data = (CreateDynamicMeshData*)evt.arg1.ptr;
		auto count = data->decrement();

		if (count == 0 && data->isValid())
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
			auto type = data->getPlayerType();
			if (type == PlayerType::Human)
			{
				createPlayerEntity(*data);
			}
			else
			{
				createActorEntity(*data);
			}
			delete(data);
		}
	}break;
	case IngameMessage::Physx_AddedActor:
	{
		CreateActorData* data = (CreateActorData*)evt.arg1.ptr;
		if (data->isValid())
		{
			auto type = data->getPlayerType();
			if (type == PlayerType::Human)
			{
				createPlayerEntity(*data);
			}
			else
			{
				createActorEntity(*data);
			}
			delete(data);
		}

	}break;
	case IngameMessage::Gpu_AddedDynamicMesh:
	{
		CreateDynamicMeshData* data = (CreateDynamicMeshData*)evt.arg1.ptr;
		auto count = data->decrement();

		if (count == 0 && data->isValid())
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
		auto evtBlock = Event::createEvent();
		physicsAspect->addBlockEvent(evtBlock);

		std::vector<Event> events;
		events.push_back(fetchEvt);
		auto task = new TaskPhysxCreateJoints(scene, physicsAspect, std::move(events), evtBlock);

		m_taskManager->pushTask(task);
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
	m_playerController.onPressedActionKey();
}

void EntityAspect::onReleasedActionKey()
{
	m_playerController.onReleasedActionKey();
}