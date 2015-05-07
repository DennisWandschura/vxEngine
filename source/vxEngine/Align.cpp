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
#include "Align.h"
#include "ComponentPhysics.h"

void Align::update(vx::float2* orientation)
{
	const f32 timeToTarget = 0.1f;

	auto rotation = m_pTarget->orientation - m_pCharacter->orientation;

	rotation.x = vx::scalarModAngle(rotation.x);

	auto rotSz = abs(rotation.x);

	if (rotSz < m_targetRadius)
		return;

	f32 targetRotation = m_maxRotation;
	if (rotSz <= m_slowRadius)
	{
		targetRotation = m_maxRotation * rotSz / m_slowRadius;
	}

	targetRotation *= rotation.x / rotSz;

	f32 angular = targetRotation - m_pCharacter->orientation.x;
	angular /= timeToTarget;

	f32 angularAccel = abs(angular);
	if (angularAccel > m_maxAngularAcceleration)
	{
		angular /= angularAccel;
		angular *= m_maxAngularAcceleration;
	}

	orientation->x = angular;
}