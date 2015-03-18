#include "SystemAspect.h"
#include "Engine.h"
#include "RenderAspect.h"

SystemAspect::SystemAspect()
	:m_window(),
	m_handleInput(nullptr)
{
}

bool SystemAspect::initialize(const vx::uint2 &windowResolution, CallbackKeyPressedFp fp, HandleInputFp inputFp)
{
	if (!m_window.initialize(L"vxEngine", windowResolution, false))
		return false;

	vx::RawInput::setCallbackKeyPressed(fp);
	m_handleInput = inputFp;

	return true;
}

void SystemAspect::shutdown()
{
	m_window.shutdown();
}

void SystemAspect::update(const F32 dt)
{
	m_window.update();

	vx::Keyboard keyboard;
	vx::RawInput::getKeyboard(keyboard);
	//m_entityAspect.handleKeyboard(keyboard);

	auto newMouse = vx::RawInput::getMouse();

	m_handleInput(newMouse, keyboard, dt);
	//m_entityAspect.handleMouse(newMouse, dt);
//	g_renderAspect.handleMouse(m_mouseOffset);
}

const vx::Window& SystemAspect::getWindow() const
{
	return m_window;
}