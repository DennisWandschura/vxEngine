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