#pragma once

class TransitionBase;
class Action;

#include <vector>

class State
{
	Action* m_actions{ nullptr };
	std::vector<TransitionBase*> m_transitions;

public:
	State() = default;
	State(Action* actions, std::vector<TransitionBase*> &&transitions) :m_actions(actions), m_transitions(transitions){}

	void addTransition(TransitionBase* p)
	{
		return m_transitions.push_back(p);
	}

	void setAction(Action *p)
	{
		m_actions = p;
	}

	const std::vector<TransitionBase*>& getTransitions() const { return m_transitions; }
	Action* getAction() const { return m_actions; }
};