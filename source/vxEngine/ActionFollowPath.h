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

namespace Component
{
	struct Actor;
}

struct EntityActor;
class ActorData;

#include "Action.h"
#include "Arrive.h"
#include <vector>

class ActionFollowPath : public Action
{
	Component::Input* m_pInput{ nullptr };
	Arrive m_arrive;
	ActorData* m_pData{ nullptr };
	Component::Actor* m_pActor{ nullptr };
	U8 m_update{1};

public:
	ActionFollowPath(Component::Input* pInput, EntityActor* entity, Component::Actor* pActor);

	void run() override;
	bool isComplete() const override;

	void updateTargetPosition();
};