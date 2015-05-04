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
#include "ActionGoToPosition.h"
#include "ComponentInput.h"
#include "Locator.h"
#include "EventManager.h"
#include "Event.h"
#include "EventsAI.h"
#include "EventTypes.h"

ActionGoToPosition::ActionGoToPosition(Component::Input* pInput, EntityActor* entity)
	:m_arrive(entity, pInput)
{
}

void ActionGoToPosition::setTarget(const vx::float3 &targetPos)
{
	m_arrive.setTarget(targetPos);
}

void ActionGoToPosition::run()
{
	if (m_arrived == 0)
	{
		SteeringOutput out;
		if (m_arrive.getSteering(&out) == 0)
		{
			m_arrived = 1;

			/*Event evt;
			evt.type = EventTypes::AI_Event;
			evt.filter = m_pActor->evtMask;
			evt.code = (U32)AIEvent::Reached_Destination;

			auto pEvtManager = Locator::getEventManager();
			pEvtManager->addEvent(evt);*/

			printf("arrived\n");
		}

		m_arrive.getInputComponent()->velocity = out.velocity;
	}
}

bool ActionGoToPosition::isComplete() const
{
	return (m_arrived != 0);
}