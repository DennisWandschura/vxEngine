#pragma once

class Engine;
class RenderAspect;
class EntityAspect;
class EngineConfig;

#include <vxLib\Window.h>
#include <vxLib\RawInput.h>

typedef void(*CallbackKeyPressedFp)(U16 key);
typedef void(*HandleInputFp)(const vx::Mouse &m, const vx::Keyboard &k, F32 dt);

class SystemAspect
{
	vx::Window m_window;
	HandleInputFp m_handleInput;

public:
	SystemAspect();

	bool initialize(const EngineConfig &config, CallbackKeyPressedFp fp, HandleInputFp inputFp);
	void shutdown();

	void update(const F32 dt);

	const vx::Window& getWindow() const;
};