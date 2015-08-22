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
	class TaskManager;
}

class PhysicsAspect;
class RenderAspect;
struct EntityHuman;
class Scene;
class CreateActorData;
class MeshInstance;
class CreateDynamicMeshData;

#include <vxEngineLib/MessageListener.h>
#include "PlayerController.h"
#include <vxLib/math/Vector.h>
#include <vxLib/Allocator/StackAllocator.h>
#include "QuadTree.h"
#include "ComponentActionManager.h"
#include "ComponentActorManager.h"
#include "Entity.h"

enum class PlayerType : u32;
enum class FileType : u8;

class EntityAspect : public vx::MessageListener
{
	struct ColdData;

	EntityHuman* m_entityHuman;
	EntityHuman m_entityHumanData;
	PlayerController m_playerController;
	ComponentActionManager m_componentActionManager;
	ComponentActorManager m_componentActorManager;
	vx::Pool<EntityActor> m_poolEntityActor;
	vx::Pool<EntityDynamic> m_poolEntityDynamic;
	QuadTree m_quadTree;
	vx::StackAllocator m_allocator;
	vx::TaskManager* m_taskManager;
	std::unique_ptr<ColdData> m_coldData;

	void createActorEntity(const CreateActorData &data);

	void createDynamicMesh(const CreateDynamicMeshData &data);

	//////////////////

	void handleFileEvent(const vx::Message &evt);
	void handleIngameMessage(const vx::Message &evt);

	void updateEntityActor(f32 dt);
	void updateEntityDynamic(f32 dt);

public:
	EntityAspect();
	~EntityAspect();

	//////////////////

	bool initialize(vx::StackAllocator* pAllocator, vx::TaskManager* taskManager, vx::AllocationProfiler* allocManager);
	void shutdown();

	void builEntityQuadTree();

	//////////////////

	void update(f32 dt, ActionManager* actionManager);

	//////////////////

	void createPlayerEntity(const vx::float3 &position);

	void handleMessage(const vx::Message &evt);

	void onPressedActionKey();

	void onReleasedActionKey();
};