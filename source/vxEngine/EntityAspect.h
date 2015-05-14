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
class RenderAspect;
struct EntityActor;
class Scene;

#include "EventListener.h"
#include "Pool.h"
#include "PlayerController.h"
#include "ComponentsForward.h"
#include <vxLib/math/Vector.h>
#include <vxLib/Allocator/StackAllocator.h>

enum class PlayerType : u32;
enum class FileType : u8;

class EntityAspect : public EventListener
{
	struct ColdData
	{
		const Scene* m_pCurrentScene{ nullptr };
		EntityActor* m_pPlayer{ nullptr };

		Pool<Component::Actor> m_poolActor;
	};

	PlayerController m_playerController;
	Pool<Component::Render> m_poolRender;
	Pool<Component::Input> m_poolInput;
	PhysicsAspect &m_physicsAspect;
	Pool<EntityActor> m_poolEntity;
	RenderAspect &m_renderAspect;
	vx::StackAllocator m_allocator;
	std::unique_ptr<ColdData> m_coldData;

	Component::Actor* createComponentActor(u16 entityIndex, EntityActor* entity, Component::Input* componentInput, u16* actorIndex);
	void createComponentPhysics(const vx::float3 &position, u16 entityIndex, f32 height);

	void createActorEntity(const vx::float3 &position, f32 height, u32 gpuIndex);

	//////////////////

	void handleFileEvent(const Event &evt);
	void handleIngameEvent(const Event &evt);

	//////////////////

public:
	EntityAspect(PhysicsAspect &physicsAspect, RenderAspect &renderAspect);

	//////////////////

	bool initialize(vx::StackAllocator* pAllocator);
	void shutdown();

	//////////////////

	void updateInput(f32 dt);

	// after physics->fetch
	void updatePhysics_linear(f32 dt);

	// updates camera to player position and orientation
	void updatePlayerPositionCamera();
	// updates transform to actor position and orientation
	void updateActorTransforms();

	//////////////////

	void createPlayerEntity(const vx::float3 &position);

	void handleEvent(const Event &evt);

	//////////////////

	EntityActor* getPlayer(){ return m_coldData->m_pPlayer; }

	Component::Input& getComponentInput(u16 i);

	const Pool<Component::Actor>& getActorPool() const { return m_coldData->m_poolActor; }
	const Pool<EntityActor>& getEntityPool() const { return m_poolEntity; }
};