#include "ActionRequestPath.h"
#include "Locator.h"
#include "EventManager.h"
#include "Event.h"
#include "EventsAI.h"
#include "EventTypes.h"
#include "ComponentActor.h"

ActionRequestPath::ActionRequestPath(Component::Actor* p)
	:m_pActor(p)
{

}

void ActionRequestPath::run()
{
	Event evt;
	evt.type = EventType::AI_Event;
	evt.filter = m_pActor->evtMask;
	evt.code = (U32)AIEvent::Request_Path;
	evt.arg1 = m_pActor;

	auto pEventManager = Locator::getEventManager();
	pEventManager->addEvent(evt);
}