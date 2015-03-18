#pragma once

class State;
class Action;

#include <vector>

class StateMachine
{
	std::vector<State*> m_states;
	State* m_pInitialState;
	State* m_pCurrentState;

public:
	StateMachine();
	StateMachine(State* pInitialState, std::vector<State*> &&states);

	Action* update();

	void setInitialState(State* pState);
	void addState(State* pState);
};