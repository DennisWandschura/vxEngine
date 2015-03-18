#include "ActionManager.h"
#include "Action.h"

void ActionManager::scheduleAction(Action* p)
{
	m_queue.push_back(p);
}

void ActionManager::update()
{
	std::vector<Action*> tmp;
	tmp.reserve(m_queue.size());

	for (auto &it : m_queue)
	{
		//if (it->getPriority() <= )
		it->run();

		if (!it->isComplete())
		{
			tmp.push_back(it);
		}
	}

	m_queue.swap(tmp);
}