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
#include "SystemAspect.h"
#include "Engine.h"
#include <vxEngineLib/EngineConfig.h>

SystemAspect::SystemAspect()
	:m_window(),
	m_handleInput(nullptr)
{
}

bool SystemAspect::initialize(const EngineConfig &config, CallbackKeyPressedFp fp, HandleInputFp inputFp)
{
	if (!m_window.initialize(L"vxEngine", config.m_resolution, false))
		return false;

	vx::RawInput::setCallbackKeyPressed(fp);
	m_handleInput = inputFp;

	return true;
}

void SystemAspect::shutdown()
{
	m_window.shutdown();
}

void SystemAspect::update(const f32 dt)
{
	m_window.update();

	vx::Keyboard keyboard;
	vx::RawInput::getKeyboard(keyboard);
	//m_entityAspect.handleKeyboard(keyboard);

	auto newMouse = vx::RawInput::getMouse();

	if (m_handleInput)
		m_handleInput(newMouse, keyboard, dt);
	//m_entityAspect.handleMouse(newMouse, dt);
//	g_renderAspect.handleMouse(m_mouseOffset);
}

const vx::Window& SystemAspect::getWindow() const
{
	return m_window;
}