#pragma once

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

namespace vx
{
	class MessageManager;
}

#include <vxLib/math/Vector.h>
#include "Action.h"
#include <vxLib/StringID.h>
#include <vxEngineLib/CpuTimer.h>

class ActionPlaySound : public Action
{
	vx::StringID m_sid;
	const vx::float3* m_position;
	vx::MessageManager* m_msgManager;
	CpuTimer m_timer;
	f32 m_elapsedTime;
	f32 m_time;

public:
	ActionPlaySound(const vx::StringID &sid, vx::MessageManager* msgManager, f32 time, const vx::float3* position);
	~ActionPlaySound();

	void run() override;
};