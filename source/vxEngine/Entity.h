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

namespace physx
{
	class PxController;
	class PxRigidDynamic;
}

namespace vx
{
	struct TransformGpu;
}

class RenderAspectInterface;
class PhysicsAspect;

#include <vxLib/math/Vector.h>

struct EntityHuman
{
	vx::float4 m_qRotation;
	vx::float3 m_position;
	f32 m_footPositionY;
	physx::PxController* m_controller;
	vx::float2 m_orientation;
	vx::float4 m_velocity;

	void update(f32 dt);
};

struct EntityActor
{
	vx::float4 m_qRotation;
	vx::float3 m_position;
	f32 m_footPositionY;
	physx::PxController* m_controller;
	u16 m_gpuIndex;
	u8 m_actorComponentIndex;
	vx::float2 m_orientation;// { vx::VX_PIDIV2, 0 };
	vx::float4 m_velocity;

	void update(f32 dt, PhysicsAspect* physicsAspect, vx::TransformGpu* transforms, u32* indices, u32 index);
};

struct EntityDynamic
{
	vx::float4 m_qRotation;
	vx::float3 m_position;
	u16 m_gpuIndex;
	u16 m_actionComponentIndex;
	physx::PxRigidDynamic* m_rigidDynamic;

	void update(f32 dt, vx::TransformGpu* transforms, u32* indices, u32 index);
};