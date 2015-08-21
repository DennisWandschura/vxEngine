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
#include <vxEngineLib/Locator.h>
#include "PhysicsAspect.h"

CollisionAvoidance::~CollisionAvoidance()
{

}

bool CollisionAvoidance::getSteering(EntityActor* currentEntity, const vx::float3 &currentPosition, const vx::float4a &inVelocity, vx::float4a* outVelocity)
{
	QuadTreeData data[2];

	u32 count = 0;
	m_quadTree->getData(currentEntity, currentPosition, m_queryRadius, 2, &count, data);

	if (count == 0)
		return false;

	vx::float4a currentVelocity(inVelocity.x, inVelocity.y, inVelocity.z, 0.0f);
	vx::float4a tmpPosition(currentPosition.x, currentPosition.y, currentPosition.z, 0.0f);
	//vx::float3 currentVelocity(inVelocity.x, inVelocity.y, inVelocity.z);

	auto actorRadius2 = m_actorRadius * 2;

	f32 targetDistance = FLT_MAX;
	vx::float4a targetPosition;
	//vx::float3 targetRelativeVelocity;
	//vx::float3 targetRelativePosition;
	//f32 targetTimeToCollision;
	bool found = false;
	for (u32 i = 0; i < count; ++i)
	{
		//if (currentEntity == data[i].entity)
		//	continue;
		vx::float4a otherPositon(data[i].position.x, data[i].position.y, data[i].position.z, 0.0f);
		vx::float4a otherVelocity(data[i].velocity.x, data[i].velocity.y, data[i].velocity.z, 0.0f);

		auto relativePos = _mm_sub_ps(otherPositon, tmpPosition);
		vx::float4a length = vx::length3(relativePos);
		auto currentDistance = length.x - actorRadius2;

		auto relativeVelocity = _mm_sub_ps(otherVelocity, currentVelocity);
		//auto len = vx::length3(relativeVelocity);
		//auto timeToCollision = -vx::dot(relativePos, relativeVelocity) / (len * len);

		auto currentDirToTarget = vx::normalize3(relativePos);
		vx::float4a tmp = vx::dot3(vx::normalize3(relativeVelocity), currentDirToTarget);
		f32 ttt = -tmp.x;

		if (ttt > 0.0f && currentDistance < targetDistance)
		{
			targetPosition = otherPositon;
			targetDistance = currentDistance;
			//targetRelativePosition = relativePos;
			//targetRelativeVelocity = relativeVelocity;
			//targetTimeToCollision = timeToCollision;
			found = true;
		}
	}

	if (found)
	{
		const __m128 upDir = { 0, 1, 0, 0 };

		currentVelocity = vx::normalize3(currentVelocity);

		auto tangent = vx::cross3(upDir, currentVelocity);

		auto absTangent = vx::abs(tangent);

		auto maxExtend = _mm_shuffle_ps(absTangent, absTangent, _MM_SHUFFLE(0, 0, 0, 0));
		auto ty = _mm_shuffle_ps(absTangent, absTangent, _MM_SHUFFLE(1, 1, 1, 1));
		maxExtend = _mm_max_ps(maxExtend, ty);
		auto tz = _mm_shuffle_ps(absTangent, absTangent, _MM_SHUFFLE(2, 2, 2, 2));
		maxExtend = _mm_max_ps(maxExtend, tz);
		auto mask = _mm_cmpeq_ps(absTangent, maxExtend);

		tangent = _mm_and_ps(tangent, mask);
		tangent = vx::normalize3(tangent);

		auto otherTangent = vx::negate(tangent);

		auto physicsAspect = Locator::getPhysicsAspect();
		vx::float3 hitPosition;
		f32 distance0 = FLT_MAX;
		f32 distance1 = FLT_MAX;

		physicsAspect->raycast_static(targetPosition, tangent, 2.0f, &hitPosition, &distance0);
		physicsAspect->raycast_static(targetPosition, otherTangent, 2.0f, &hitPosition, &distance1);

		auto finalVelocity = (distance0 >= distance1) ? tangent : otherTangent;

		/*if (targetDistance <= actorRadius2)
		{
		//	targetRelativePosition = tangent;//(distance0 >= distance1) ? tangent : otherTangent;
		}
		else*/
		if (targetDistance > actorRadius2)
		{
			auto weight = targetDistance / m_queryRadius;
			auto otherWeight = 1.0f - weight;

			__m128 tmp0 = { otherWeight, otherWeight, otherWeight, 0.0f };
			__m128 tmp1 = { weight, weight, weight, 0.0f };

			finalVelocity = _mm_mul_ps(finalVelocity, tmp0);
			finalVelocity = _mm_fmadd_ps(currentVelocity, tmp1, finalVelocity);

			//finalVelocity = finalVelocity * (1.0f - weight) + velocity * weight;
		}

		__m128 vMaxAccel = { m_maxAccel, m_maxAccel, m_maxAccel, 0.0f};

		*outVelocity = _mm_mul_ps(finalVelocity, vMaxAccel);
	}

	return found;
}