#pragma once

namespace vx
{
	struct Keyboard;
	struct Mouse;
	class Camera;
}

struct Entity;
class EntityAspect;
class PhysicsAspect;

#include <vxLib/types.h>

class PlayerController
{
	vx::Camera& m_camera;

public:
	explicit PlayerController(vx::Camera &camera);

	void updatePlayerHuman(Entity* pPlayer, EntityAspect &entityAspect);

	void handleKeyboard(Entity* pPlayer, const vx::Keyboard &keyboard, EntityAspect &entityAspect);
	void handleMouse(Entity* pPlayer, const vx::Mouse &mouse, const F32 dt, EntityAspect &entityAspect);

	void keyPressed(U16 key, EntityAspect &entityAspect);
};