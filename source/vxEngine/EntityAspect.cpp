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

namespace
{
	template<typename T>
	void createPool(U16 capacity, U8 alignment, vx::StackAllocator* pAllocator, Pool<T> *pPool)
	{
		pPool->initialize(pAllocator->allocate(sizeof(T) * capacity, alignment), capacity);
	}
}

EntityAspect::EntityAspect(PhysicsAspect &physicsAspect, vx::Camera &camera, FileAspect &fileAspect, RenderAspect &renderAspect)
	:m_playerController(camera),
	m_physicsAspect(physicsAspect),
	m_fileAspect(fileAspect),
	m_renderAspect(renderAspect)
{
}

bool EntityAspect::initialize(vx::StackAllocator* pAllocator)
{
	createPool(g_maxEntities, 16, pAllocator, &m_poolPhysics);
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
	m_poolPhysics.release();
}

Component::Physics* EntityAspect::createComponentPhysics(const vx::float3 &position, U16 entityIndex, F32 height, U16* index)
{
	auto pPhysics = m_poolPhysics.createEntry(index);
	auto pActor = m_physicsAspect.createActor(position, height);

	pPhysics->position = position;
	pPhysics->entityIndex = entityIndex;
	pPhysics->pRigidActor = pActor;

	return pPhysics;
}

Component::Actor* EntityAspect::createComponentActor(U16 entityIndex, U16* actorIndex)
{
	auto pActor = m_poolActor.createEntry(actorIndex);
	pActor->entityIndex = entityIndex;

	// add empty initial action
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

		createComponentPhysics(position, entityIndex, g_heightStanding, &m_pPlayer->physics);
	}
}

void EntityAspect::createActorEntity(const vx::float3 &position, const vx::StringID64 &actor, F32 height)
{
	U16 entityIndex;
	auto pEntity = m_poolEntity.createEntry(&entityIndex);
	auto pInput = m_poolInput.createEntry(&pEntity->input);
	pInput->entityIndex = entityIndex;

	auto pPhysics = createComponentPhysics(position, entityIndex, height, &pEntity->physics);

	auto &actors = m_pCurrentScene->getActors();
	auto itActor = actors.find(actor);

	vx::Transform transform;
	transform.m_translation = position;

	auto gpuIndex = m_renderAspect.addActorToBuffer(transform, itActor->mesh, itActor->material, m_pCurrentScene);
	//auto gpuIndex = m_renderAspect.getActorGpuIndex();
	auto pRender = m_poolRender.createEntry(&pEntity->render);
	pRender->entityIndex = entityIndex;
	pRender->gpuIndex = gpuIndex;

	auto pActor = createComponentActor(entityIndex, &pEntity->actor);

	F32 halfHeight = height * 0.5f;
	EntityFactory::create(m_pNavGraph, pActor, pInput, pPhysics, halfHeight, &m_poolAllocatorPath);

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
		auto entity = m_poolEntity[p->entityIndex];
		auto &physics = m_poolPhysics[entity.physics];

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

		physics.orientation = p->orientation;

		//__m128 vSpeed = { 0, 0, 0, 0 };
		//vSpeed = _mm_fmadd_ps(vSpeed, runOffset, move);

		__m128 vVelocity = { p->velocity.x, 0, p->velocity.z, 0.0f };
		//vOffset = _mm_fmadd_ps(vOffset, vSpeed, gravity);
		vVelocity = _mm_add_ps(vVelocity, vGravity);
		//vVelocity = _mm_mul_ps(vVelocity, vDt);

		m_physicsAspect.move(vVelocity, dt, physics.pRigidActor);

		p = m_poolInput.next_nocheck(p);
	}
}

void EntityAspect::updatePhysics_linear(F32 dt)
{
	UNREFERENCED_PARAMETER(dt);

	auto p = m_poolPhysics.first();
	while (p != nullptr)
	{
		auto position = p->pRigidActor->getPosition();
		p->position.x = position.x;
		p->position.y = position.y;
		p->position.z = position.z;

		m_pInfluenceMap->updateActor(dt, p->position);

		p = m_poolPhysics.next_nocheck(p);
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
	std::pair<U16, U16>* pIndices_Sorted = (std::pair<U16, U16>*)m_allocator.allocate(sizeof(std::pair<U16, U16>) * count, 4);
	vx::TransformGpu* pTransforms = (vx::TransformGpu*)m_allocator.allocate(sizeof(vx::TransformGpu) * count, 16);
	U32 index = 0;

	auto p = m_poolRender.first();
	while (p != nullptr)
	{
		auto entity = m_poolEntity[p->entityIndex];
		auto physics = m_poolPhysics[entity.physics];

		__m128 v = { physics.orientation.y, physics.orientation.x, 0 , 0};
		v = vx::QuaternionRotationRollPitchYawFromVector(v);
		auto packedRotation = packQRotation(v);

		vx::TransformGpu transform;
		transform.translation = physics.position;
		transform.scaling = 1.0f;
		transform.packedQRotation = packedRotation;

		//pIndices[index] = p->gpuIndex;
		pIndices_Sorted[index] = std::make_pair(p->gpuIndex, index);
		pTransforms[index] = transform;
		++index;

		//m_renderAspect.updateTransform(p->gpuIndex, transform);

		p = m_poolRender.next_nocheck(p);
	}

	std::sort(pIndices_Sorted, pIndices_Sorted + count, [](const std::pair<U16, U16> &l, const std::pair<U16, U16> &r)
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

		m_renderAspect.updateTransforms(pTransforms + offset, batchStart, batchCount);

		count -= batchCount;
		offset += batchCount;
	}

	//m_renderAspect.updateTransforms();
}

Component::Physics& EntityAspect::getComponentPhysics(U16 i)
{
	return m_poolPhysics[i];
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
				createActorEntity(it.position, it.sid, 2.0f);
			}
		}

	}break;
	case IngameEvent::Created_InfluenceMap:
	{
		m_pInfluenceMap = (InfluenceMap*)evt.arg1.ptr;
	}break;
	default:
		break;
	}
}