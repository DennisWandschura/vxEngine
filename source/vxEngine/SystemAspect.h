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