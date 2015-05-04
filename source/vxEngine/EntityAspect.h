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
#pragma once

namespace vx
{
	class StackAllocator;
	struct Keyboard;
	struct Mouse;
	class Camera;
}

class PhysicsAspect;
class FileAspect;
class RenderAspect;
struct Spawn;
struct EntityActor;
class Scene;
class NavGraph;
class InfluenceMap;

#include "EventListener.h"
#include "Pool.h"
#include "LoadFileCallback.h"
#include "PlayerController.h"
#include <atomic>
#include <vector>
#include "ComponentsForward.h"
#include <vxLib/math/Vector.h>
#include <vxLib/Allocator/StackAllocator.h>
#include <vxLib/Allocator/PoolAllocator.h>

enum class PlayerType : U32;
enum class FileType : U8;

class EntityAspect : public EventListener
{
	static const  auto s_maxNavNodes = 50u;

	PlayerController m_playerController;
	PhysicsAspect &m_physicsAspect;
	//Pool<Component::Physics> m_poolPhysics;
	Pool<Component::Input> m_poolInput;
	Pool<Component::Render> m_poolRender;
	Pool<Component::Actor> m_poolActor;
	Pool<EntityActor> m_poolEntity;
	const NavGraph* m_pNavGraph{ nullptr };
	InfluenceMap* m_pInfluenceMap{nullptr};
	EntityActor* m_pPlayer{ nullptr };
	FileAspect &m_fileAspect;
	RenderAspect &m_renderAspect;
	vx::StackAllocator m_allocator;
	const Scene* m_pCurrentScene{ nullptr };
	vx::PoolAllocator m_poolAllocatorPath;

	Component::Actor* createComponentActor(U16 entityIndex, U16* actorIndex);
	void createComponentPhysics(const vx::float3 &position, U16 entityIndex, F32 height);

	void spawnPlayer(const vx::float3 &position, const Component::Physics &p);

	void createActorEntity(const vx::float3 &position, F32 height, U32 gpuIndex);

	//////////////////

	void handleFileEvent(const Event &evt);
	void handleIngameEvent(const Event &evt);

	//////////////////

public:
	EntityAspect(PhysicsAspect &physicsAspect, FileAspect &fileAspect, RenderAspect &renderAspect);

	//////////////////

	bool initialize(vx::StackAllocator* pAllocator);
	void shutdown();

	//////////////////

	void updateInput(F32 dt);

	// after physics->fetch
	void updatePhysics_linear(F32 dt);

	// updates camera to player position and orientation
	void updatePlayerPositionCamera();
	// updates transform to actor position and orientation
	void updateActorTransforms();

	//////////////////

	void createPlayerEntity(const vx::float3 &position);

	//////////////////

	void handleKeyboard(const vx::Keyboard &keyboard);
	void handleMouse(const vx::Mouse &mouse, const F32 dt);
	void keyPressed(U16 key);

	//////////////////

	void handleEvent(const Event &evt);

	//////////////////

	EntityActor* getPlayer(){ return m_pPlayer; }

	Component::Input& getComponentInput(U16 i);

	const Pool<Component::Actor>& getActorPool() const { return m_poolActor; }
	const Pool<EntityActor>& getEntityPool() const { return m_poolEntity; }
};