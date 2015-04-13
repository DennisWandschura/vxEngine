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
struct Entity;
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
	Pool<Component::Physics> m_poolPhysics;
	Pool<Component::Input> m_poolInput;
	Pool<Component::Render> m_poolRender;
	Pool<Component::Actor> m_poolActor;
	const NavGraph* m_pNavGraph{ nullptr };
	InfluenceMap* m_pInfluenceMap{nullptr};
	Pool<Entity> m_poolEntity;
	Entity* m_pPlayer{ nullptr };
	FileAspect &m_fileAspect;
	RenderAspect &m_renderAspect;
	vx::StackAllocator m_allocator;
	const Scene* m_pCurrentScene{ nullptr };
	vx::PoolAllocator m_poolAllocatorPath;

	Component::Physics* createComponentPhysics(const vx::float3 &position, U16 entityIndex, F32 height, U16* index);
	Component::Actor* createComponentActor(U16 entityIndex, U16* actorIndex);

	void spawnPlayer(const vx::float3 &position, const Component::Physics &p);

	void createActorEntity(const vx::float3 &position, const vx::StringID64 &actor, F32 height);

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

	Entity* getPlayer(){ return m_pPlayer; }

	Component::Physics& getComponentPhysics(U16 i);
	Component::Input& getComponentInput(U16 i);

	const Pool<Component::Physics>& getPhysicsPool() const { return m_poolPhysics; }
	const Pool<Component::Actor>& getActorPool() const { return m_poolActor; }
	const Pool<Entity>& getEntityPool() const { return m_poolEntity; }
};