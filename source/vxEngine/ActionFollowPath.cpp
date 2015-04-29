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
		evt.arg1 = m_pActor;

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