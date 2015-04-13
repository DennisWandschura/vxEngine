#pragma once

namespace vx
{
	struct Keyboard;
	struct Mouse;
	class Camera;
}

struct Entity;
class EntityAspect;
class RenderAspect;

#include <vxLib/types.h>

class PlayerController
{
	RenderAspect* m_pRenderAspect;

public:
	explicit PlayerController(RenderAspect* renderAspect);

	void updatePlayerHuman(Entity* pPlayer, EntityAspect &entityAspect);

	void handleKeyboard(Entity* pPlayer, const vx::Keyboard &keyboard, EntityAspect &entityAspect);
	void handleMouse(Entity* pPlayer, const vx::Mouse &mouse, const F32 dt, EntityAspect &entityAspect);

	void keyPressed(U16 key, EntityAspect &entityAspect);
};