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

class Condition;
class Action;
class State;

#include <vxLib/types.h>
#include <vector>

class TransitionBase
{
protected:
	~TransitionBase(){}

public:
	virtual u8 isTriggered() = 0;
	virtual State* getTargetState() const = 0;
	virtual Action* getAction() const = 0;
};

class Transition : public TransitionBase
{
	Action* m_actions;
	Condition* m_pCondition;
	State* m_pTargetState;

public:
	Transition(Condition* pCondition, State* pTargetState, Action* actions = nullptr) :
		m_actions(actions), m_pCondition(pCondition), m_pTargetState(pTargetState){}

	~Transition(){}

	u8 isTriggered() override;
	State* getTargetState() const override { return m_pTargetState; }
	Action* getAction() const override { return m_actions; }
};