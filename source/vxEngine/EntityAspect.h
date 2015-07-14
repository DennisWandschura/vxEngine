#pragma once
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

namespace vx
{
	class StackAllocator;
	struct Keyboard;
	struct Mouse;
	class Camera;
	struct StringID;
	class AllocationProfiler;
}

class PhysicsAspect;
class RenderAspect;
struct Entity;
class Scene;
class CreateActorData;
class MeshInstance;
class TaskManager;

#include <vxEngineLib/EventListener.h>
#include "PlayerController.h"
#include <vxLib/math/Vector.h>
#include <vxLib/Allocator/StackAllocator.h>
#include "QuadTree.h"
#include "ComponentUsableManager.h"
#include "ComponentRenderManager.h"
#include "ComponentPhysicsManager.h"
#include "ComponentInputManager.h"
#include "ComponentActorManager.h"

enum class PlayerType : u32;
enum class FileType : u8;

class EntityAspect : public vx::EventListener
{
	struct ColdData
	{
		const Scene* m_pCurrentScene{ nullptr };
		Entity* m_pPlayer{ nullptr };
	};

	PlayerController m_playerController;
	ComponentInputManager m_componentInputManager;
	ComponentPhysicsManager m_componentPhysicsManager;
	ComponentRenderManager m_componentRenderManager;
	ComponentUsableManager m_componentUsableManager;
	ComponentActorManager m_componentActorManager;
	vx::Pool<Entity> m_poolEntity;
	QuadTree m_quadTree;
	vx::StackAllocator m_allocator;
	TaskManager* m_taskManager;
	std::unique_ptr<ColdData> m_coldData;

	void createActorEntity(const CreateActorData &data);

	void createEntityUsable(const MeshInstance &instance, u32 gpuIndex);

	//////////////////

	void handleFileEvent(const vx::Event &evt);
	void handleIngameEvent(const vx::Event &evt);

public:
	EntityAspect();

	//////////////////

	bool initialize(vx::StackAllocator* pAllocator, TaskManager* taskManager, vx::AllocationProfiler* allocManager);
	void shutdown();

	void builEntityQuadTree();

	//////////////////

	void update(f32 dt, ActionManager* actionManager);

	//////////////////

	void createPlayerEntity(const vx::float3 &position);

	void handleEvent(const vx::Event &evt);

	void onPressedActionKey();

	//////////////////

	Entity* getPlayer(){ return m_coldData->m_pPlayer; }

	Component::Input& getComponentInput(u16 i);

	const vx::Pool<Entity>& getEntityPool() const { return m_poolEntity; }
};