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
#include "Seek.h"
#include <vxEngineLib/Entity.h>

Seek::Seek(Entity* entity, const vx::float3 &target, f32 maxAccel)
	:m_entity(entity),
	m_targetPosition(target),
	m_maxAcceleration(maxAccel)
{
}

void Seek::setTarget(const vx::float3 &target)
{
	m_targetPosition = target;
}

bool Seek::getSteering(SteeringOutput* output)
{
	auto dir = m_targetPosition - m_entity->position;
	dir = vx::normalize3(dir);

	dir *= m_maxAcceleration;

	output->velocity = dir;
	output->angular = 0.0f;

	return true;
}