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
#include "ActionFollowPath.h"
#include "ComponentInput.h"
#include "Locator.h"
#include "EventManager.h"
#include "Event.h"
#include "EventsAI.h"
#include "EventTypes.h"
#include "NavGraph.h"
#include "NavNode.h"
#include "ComponentActor.h"

ActionFollowPath::ActionFollowPath(Component::Input* pInput, EntityActor* entity, Component::Actor* pActor)
	:m_arrive(entity, pInput),
	m_pData(pActor->data.get()),
	m_pActor(pActor)
{
}

void ActionFollowPath::run()
{
	if (m_update != 0)
	{
		updateTargetPosition();
		m_update = 0;
	}

	SteeringOutput out;
	if (m_arrive.getSteering(&out) == 0)
	{
		m_pData->path.pop_back();

		updateTargetPosition();
	}

	m_pInput->velocity = out.velocity;
}

bool ActionFollowPath::isComplete() const
{
	if (m_pData->path.size() == 0)
	{
		m_pActor->flags |= Component::Actor::WaitingForOrders;
		m_pActor->flags ^= (Component::Actor::HasPath | Component::Actor::HasDestination);

		Event evt;
		evt.type = EventType::AI_Event;
		evt.filter = m_pActor->evtMask;
		evt.code = (U32)AIEvent::Reached_Destination;
		evt.arg1.ptr = m_pActor;

		auto pEvtManager = Locator::getEventManager();
		pEvtManager->addEvent(evt);

		return true;
	}

	return false;
}

void ActionFollowPath::updateTargetPosition()
{
	if (m_pData->path.size() != 0)
	{
		auto dest = m_pData->path.back();
		dest.y = 1.445f;

		m_arrive.setTarget(dest);
	}
}