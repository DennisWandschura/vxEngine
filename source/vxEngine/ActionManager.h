#pragma once

class Action;

#include <vector>

class ActionManager
{
	std::vector<Action*> m_queue{};
	std::vector<Action*>* m_active{};

public:
	ActionManager() = default;

	void scheduleAction(Action* p);

	void update();
};