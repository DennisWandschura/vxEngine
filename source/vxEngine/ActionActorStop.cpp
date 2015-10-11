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
#include "ActionActorStop.h"
#include "ComponentActor.h"
#include "ActionFollowPath.h"
#include "Entity.h"

ActionActorStop::ActionActorStop(EntityActor* entity, Component::Actor* actorData, ActionFollowPath* actionFollowPath)
	:m_entity(entity),
	m_actor(actorData),
	m_actionFollowPath(actionFollowPath)
{

}

ActionActorStop::~ActionActorStop()
{

}

void ActionActorStop::run()
{
	m_actor->m_data->path.clear();
	m_actor->m_followingPath = 0;

	auto position = m_entity->m_position;
	position.y = m_entity->m_footPositionY;
	m_actionFollowPath->setTarget(position);
}

bool ActionActorStop::isComplete() const
{
	return true;
}