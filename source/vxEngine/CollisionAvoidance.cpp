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
#include "CollisionAvoidance.h"
#include "QuadTree.h"

CollisionAvoidance::~CollisionAvoidance()
{

}

bool CollisionAvoidance::getSteering(EntityActor* currentEntity, const vx::float3 &currentPosition, const vx::float4 &inVelocity, vx::float3* outVelocity)
{
	QuadTreeData data[2];

	u32 count = 0;
	m_quadTree->getData(currentEntity, currentPosition, m_queryRadius, 2, &count, data);

	if (count == 0)
		return false;

	vx::float3 currentVelocity(inVelocity.x, inVelocity.y, inVelocity.z);

	auto actorRadius2 = m_actorRadius * 2;

	f32 targetDistance = FLT_MAX;
	vx::float3 directionToTarget;
	vx::float3 targetRelativeVelocity;
	vx::float3 targetRelativePosition;
	f32 targetTimeToCollision;
	bool found = false;
	for (u32 i = 0; i < count; ++i)
	{
		//if (currentEntity == data[i].entity)
		//	continue;

		auto relativePos = data[i].position - currentPosition;
		auto currentDistance = vx::length(relativePos) - actorRadius2;

		auto relativeVelocity = data[i].velocity - currentVelocity;
		auto len = vx::length(relativeVelocity);
		auto timeToCollision = -vx::dot(relativePos, relativeVelocity) / (len * len);

		auto currentDirToTarget = vx::normalize(relativePos);
		auto tmp1 = vx::normalize(relativeVelocity);
		auto tmp = -vx::dot(currentDirToTarget, tmp1);

		if (tmp > 0.0f)
		{
			targetDistance = currentDistance;
			targetRelativePosition = relativePos;
			directionToTarget = currentDirToTarget;
			targetRelativeVelocity = relativeVelocity;
			targetTimeToCollision = timeToCollision;
			found = true;
		}
	}

	if (found)
	{
		const vx::float3 upDir = { 0, 1, 0 };
		auto tangent = vx::cross(upDir, directionToTarget);

		if (targetDistance <= actorRadius2)
		{
			/*u32 maxExtend = 0;
			if (tangent.y < tangent.x && tangent.y < tangent.z)
			{
				maxExtend = 1;
			}
			else if (tangent.z < tangent.x)
			{
				maxExtend = 2;
			}*/

			targetRelativePosition = tangent;
		}
		else
		{
			auto weight = targetDistance / m_queryRadius;

			targetRelativePosition = vx::normalize(tangent * (1.0f - weight) + directionToTarget * weight);
			
		}

		*outVelocity = targetRelativePosition * m_maxAccel;
	}

	return found;
}