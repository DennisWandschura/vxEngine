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
#include "Arrive.h"
#include "ComponentInput.h"
#include "Entity.h"

Arrive::Arrive(EntityActor* entity, Component::Input* pInput)
	:m_pEntity(entity),
	m_pInput(pInput)
{
}

u8 Arrive::getSteering(SteeringOutput* output)
{
	const f32 targetRadius = 0.25f;
//	const f32 slowRadius = 1.0f;
	//const f32 timeToTarget = 0.1f;

	auto direction = m_targetPosition - m_pEntity->position;
	auto distance = vx::length(direction);

	if (distance < targetRadius)
	{
		return 0;
	}

	auto velocity = direction / distance * m_maxSpeed;
	output->velocity = velocity;

	/*f32 targetSpeed = m_maxSpeed;

	if (distance <= slowRadius)
	{
		targetSpeed = m_maxSpeed * distance / slowRadius;
	}

	auto targetVelocity = direction / distance;
	//targetVelocity = vx::normalize(targetVelocity);
	targetVelocity *= targetSpeed;

	output->velocity = targetVelocity - m_pInput->velocity;
	output->velocity /= timeToTarget;

	auto l = vx::length(output->velocity);
	if (l > m_maxAcceleration)
	{
		output->velocity = (output->velocity / l) * m_maxAcceleration;
	}*/

	output->angular = 0.0f;

	return 1;
}

void Arrive::setTarget(const vx::float3 &targetPos)
{
	m_targetPosition = targetPos;
}

Component::Input* Arrive::getInputComponent() const
{
	return m_pInput;
}