#pragma once

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

struct Entity;
class QuadTree;

#include <vxLib/math/Vector.h>

class CollisionAvoidance
{
	const QuadTree* m_quadTree;
	f32 m_actorRadius;
	f32 m_queryRadius;
	f32 m_maxAccel;

public:
	CollisionAvoidance(const QuadTree* quadTree, f32 actorRadius, f32 queryRadius, f32 maxAccel):m_quadTree(quadTree), m_actorRadius(actorRadius), m_queryRadius(queryRadius), m_maxAccel(maxAccel){}
	~CollisionAvoidance();

	bool getSteering(Entity* currentEntity, const vx::float3 &currentPosition, const vx::float4a &inVelocity, vx::float4a* outVelocity);
};
