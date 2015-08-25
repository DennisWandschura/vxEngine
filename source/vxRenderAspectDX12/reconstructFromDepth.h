#pragma once

float reconstructCSZ(float d, float3 clipInfo) 
{
	return clipInfo[0] / (clipInfo[1] * d + clipInfo[2]);
}

float3 reconstructCSPosition(float2 S, float z, float4 projInfo) 
{
	return float3((S.xy * projInfo.xy + projInfo.zw) * z, z);
}

float3 reconstructCSPositionFromDepth(float2 S, float depth, float4 projInfo, float3 clipInfo)
{
	return reconstructCSPosition(S, reconstructCSZ(depth, clipInfo), projInfo);
}

float3 reconstructWSPositionFromDepth(float2 S, float depth, float4 projInfo, float3 clipInfo, mat4x3 cameraToWorld)
{
	return cameraToWorld * float4(reconstructCSPositionFromDepth(S, depth, projInfo, clipInfo), 1.0);
}