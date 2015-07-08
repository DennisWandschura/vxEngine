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
#include <vxEngineLib/Entity.h>
#include "ComponentInput.h"
#include "ComponentActor.h"

ActionFollowPath::ActionFollowPath(EntityActor* entity, Component::Input* componentInput, Component::Actor* actor, const QuadTree* quadTree, f32 actorRadius, f32 queryRadius)
	:m_componentInput(componentInput),
	m_entity(entity),
	m_arrive(),
	m_lookWhereYoureGoing(componentInput),
	m_actor(actor),
	m_avoidance(quadTree, actorRadius, queryRadius, 0.1f),
	m_arrived(false)
{
}

void ActionFollowPath::run()
{
	if (!m_arrived)
	{
		SteeringOutput steering;
		auto currentPosition = m_entity->position;

		auto positionFoot = currentPosition;
		positionFoot.y = m_entity->footPositionY;

		vx::float4 velocity;
		if (!m_arrive.getSteering(positionFoot, &steering))
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
			velocity.x = steering.velocity.x;
			velocity.y = steering.velocity.y;
			velocity.z = steering.velocity.z;

			m_lookWhereYoureGoing.getSteering(&steering);

			auto currentTarget = m_arrive.getTarget();

			vx::float4a outVelocity;
			auto tmpVelocity = vx::loadFloat4(velocity);
			if (m_avoidance.getSteering(m_entity, positionFoot, tmpVelocity, &outVelocity))
			{
				velocity.x += outVelocity.x;
				velocity.y += outVelocity.y;
				velocity.z += outVelocity.z;

				velocity.x *= 0.5f;
				velocity.y *= 0.5f;
				velocity.z *= 0.5f;
			}

			m_componentInput->orientation.x = steering.angular;
		}

		m_componentInput->velocity = velocity;
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