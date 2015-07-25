#pragma once

#include <vxLib/StringID.h>
#include <vxEngineLib/Transform.h>

enum class JointType : u32
{
	Revolute
};

struct JointV6
{
	vx::StringID sid0;
	vx::StringID sid1;
	JointType type;
	vx::float3 p0;
	vx::float4 q0;
	vx::float3 p1;
	vx::float4 q1;
};

struct Joint
{
	vx::StringID sid0;
	vx::StringID sid1;
	JointType type;
	vx::float3 p0;
	vx::float4 q0;
	vx::float3 p1;
	vx::float4 q1;
	vx::float2 limit;
	u32 limitEnabled;
};