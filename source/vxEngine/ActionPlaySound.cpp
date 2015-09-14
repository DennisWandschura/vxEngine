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

#include "ActionPlaySound.h"
#include <vxEngineLib/MessageManager.h>
#include <vxEngineLib/Message.h>
#include <vxEngineLib/AudioMessage.h>
#include <vxEngineLib/MessageTypes.h>

ActionPlaySound::ActionPlaySound(const vx::StringID &sid, vx::MessageManager* msgManager, f32 time)
	:m_sid(sid),
	m_msgManager(msgManager),
	m_timer(),
	m_time(time)
{

}

ActionPlaySound::~ActionPlaySound()
{

}

void ActionPlaySound::run()
{
	auto elapsedTime = m_timer.getTimeSeconds();
	if (elapsedTime >= m_time)
	{
		vx::Message msg;
		msg.arg1.u64 = m_sid.value;
		msg.code = (u32)vx::AudioMessage::PlaySound;
		msg.type = vx::MessageType::Audio;
		m_msgManager->addMessage(msg);

		m_timer.reset();
	}
}