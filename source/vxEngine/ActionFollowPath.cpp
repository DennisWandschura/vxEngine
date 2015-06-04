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
#include "ActionFollowPath.h"
#include "Entity.h"
#include "ComponentInput.h"
#include "ComponentActor.h"

ActionFollowPath::ActionFollowPath(EntityActor* entity, Component::Input* componentInput, Component::Actor* actor)
	:m_componentInput(componentInput),
	m_entity(entity),
	m_arrive(),
	m_lookWhereYoureGoing(componentInput),
	m_actor(actor),
	m_arrived(false)
{
}

void ActionFollowPath::run()
{
	if (!m_arrived)
	{
		SteeringOutput steering;
		auto currentPosition = m_entity->position;
		currentPosition.y = m_entity->footPositionY;
		if (!m_arrive.getSteering(currentPosition, &steering))
		{
			if (m_actor->m_data->path.empty())
			{
				m_actor->m_followingPath = 0;
				m_arrived = true;
			}
			else
			{
				auto nextTarget = m_actor->m_data->path.back();
				m_actor->m_data->path.pop_back();
				m_arrive.setTarget(nextTarget);
			}
		}
		else
		{
			m_componentInput->velocity.x = steering.velocity.x;
			m_componentInput->velocity.y = steering.velocity.y;
			m_componentInput->velocity.z = steering.velocity.z;

			m_lookWhereYoureGoing.getSteering(&steering);

			//memcpy(&m_componentInput->velocity, &steering.velocity, sizeof(vx::float3));

			m_componentInput->orientation.x = steering.angular;
		}
	}
}

bool ActionFollowPath::isComplete() const
{
	return m_arrived;
}

void ActionFollowPath::setTarget(const vx::float3 &target)
{
	m_arrived = false;
	m_arrive.setTarget(target);
}