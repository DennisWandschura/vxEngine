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
#pragma once

class TransitionBase;
class Action;

#include <vector>

class State
{
	std::vector<Action*> m_actions;
	std::vector<TransitionBase*> m_transitions;
	std::vector<Action*> m_entryActions;
	std::vector<Action*> m_exitActions;

public:
	State() :m_actions(), m_transitions(), m_entryActions(), m_exitActions(){}

	void addTransition(TransitionBase* p)
	{
		return m_transitions.push_back(p);
	}

	void addAction(Action *p)
	{
		m_actions.push_back(p);
	}

	void addEntryAction(Action *p)
	{
		m_entryActions.push_back(p);
	}

	void addExitAction(Action *p)
	{
		m_exitActions.push_back(p);
	}

	const std::vector<TransitionBase*>& getTransitions() const { return m_transitions; }
	const std::vector<Action*>& getActions() const { return m_actions; }
	const std::vector<Action*>& getEntryActions() const { return m_entryActions; }
	const std::vector<Action*>& getExitActions() const { return m_exitActions; }
};