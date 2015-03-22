#pragma once

struct Event;
class EventListener;

#include <vxLib/types.h>
#include <vector>
#include "SquirrelVM.h"
#include <vxLib/Container/sorted_array.h>
#include <mutex>

class EventManager
{
	std::mutex m_evtMutex;
	std::vector<Event> m_events[2];
	std::vector<std::pair<U64, EventListener*>> m_eventListeners;
	U32 m_currentReadQueue{0};
	SquirrelVM m_vm;

public:
	EventManager();
	~EventManager();

	void initialize();

	void registerListener(EventListener* ptr, U64 priority);

	void update();

	void addEvent(const Event &evt);
};