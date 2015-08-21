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

#include "ActionPlayerLookAround.h"
#include <vxLib/RawInput.h>
#include "Entity.h"

ActionPlayerLookAround::ActionPlayerLookAround(EntityHuman* player, f32 dt)
	: m_player(player),
	m_halfDt(dt / 2.0f)
{

}

ActionPlayerLookAround::~ActionPlayerLookAround()
{
}

void ActionPlayerLookAround::run()
{
	const f32 angleMax = 1.4f;
	const f32 angleMin = -1.4f;

	auto mouse = vx::RawInput::getMouse();
	auto mouseRelativePos = mouse.m_relative;
	auto mouseRelativePosFloat = static_cast<vx::float2>(mouseRelativePos);

	auto orientation = m_player->m_orientation;

	//const f32 halfDt = 0.5f * m_dt;

	orientation.x = orientation.x - mouseRelativePosFloat.x * m_halfDt;
	orientation.y = orientation.y - mouseRelativePosFloat.y * m_halfDt;

	m_player->m_orientation.x = vx::scalarModAngle(orientation.x);
	m_player->m_orientation.y = fminf(fmaxf(orientation.y, angleMin), angleMax);
}