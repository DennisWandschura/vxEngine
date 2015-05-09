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
#include "StateMachine.h"
#include "Transition.h"
#include "State.h"
#include <vxLib/Allocator/StackAllocator.h>

StateMachine::StateMachine()
	:m_states(),
	m_pInitialState(nullptr),
	m_pCurrentState(nullptr)
{
}

StateMachine::StateMachine(State* pInitialState, std::vector<State*> &&states)
	:m_states(std::move(states)),
	m_pInitialState(pInitialState),
	m_pCurrentState(pInitialState)
{

}

void StateMachine::update(Action** actions, u32* count, vx::StackAllocator* allocator)
{
	*count = 0;

	if (m_pCurrentState == nullptr)
		return;

	TransitionBase* pTriggeredTransition = nullptr;

	auto &transitions = m_pCurrentState->getTransitions();
	for (auto &it : transitions)
	{
		if (it->isTriggered())
		{
			pTriggeredTransition = it;
			break;
		}
	}

	if (pTriggeredTransition)
	{
		auto &exitActions = m_pCurrentState->getExitActions();

		auto targetState = pTriggeredTransition->getTargetState();

		auto &transitionActions = pTriggeredTransition->getActions();
		auto &entryActions = targetState->getEntryActions();
		auto targetStateActions = targetState->getActions();

		auto actionCount = exitActions.size();
		actionCount += transitionActions.size();
		actionCount += entryActions.size();
		actionCount += targetStateActions.size();

		*actions = (Action*)allocator->allocate(sizeof(Action*) * actionCount, 8);
		*count = actionCount;

		u32 index = 0;
		for (auto &it : exitActions)
		{
			actions[index++] = it;
			//actions->push_back(it);
		}

		for (auto &it : transitionActions)
		{
			actions[index++] = it;
			//actions->push_back(it);
		}

		for (auto &it : entryActions)
		{
			actions[index++] = it;
			//actions->push_back(it);
		}

		for (auto &it : targetStateActions)
		{
			actions[index++] = it;
			//actions->push_back(it);
		}

		m_pCurrentState = targetState;
	}
	else
	{
		auto &currentActions = m_pCurrentState->getActions();
		auto actionCount = currentActions.size();
		printf("%llu\n", actionCount);
		*actions = (Action*)allocator->allocate(sizeof(Action*) * actionCount, 8);
		*count = actionCount;

		u32 index = 0;
		for (auto &it : currentActions)
		{
			actions[index++] = it;
		}
	}

}

void StateMachine::setInitialState(State* pState)
{
	m_pInitialState = pState;
	m_pCurrentState = pState;
}

void StateMachine::addState(State* pState)
{
	m_states.push_back(pState);
}