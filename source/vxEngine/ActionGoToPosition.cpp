#include "ActionGoToPosition.h"
#include "ComponentInput.h"
#include "Locator.h"
#include "EventManager.h"
#include "Event.h"
#include "EventsAI.h"
#include "EventTypes.h"

ActionGoToPosition::ActionGoToPosition(Component::Input* pInput, Component::Physics* pPhysics)
	:m_pInput(pInput),
	m_arrive(pPhysics, pInput)
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

		m_pInput->velocity = out.velocity;
	}
}

bool ActionGoToPosition::isComplete() const
{
	return (m_arrived != 0);
}