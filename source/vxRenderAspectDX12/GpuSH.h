#ifdef _VX_WINDOWS
#pragma once
#endif

#ifndef _GPUSH_H
#define _GPUSH_H

half4 SHRotate(const in half3 vcDir, const in half2 vZHCoeffs)
{
	// compute sine and cosine of thetta angle
	// beware of singularity when both x and y are 0 (no need to rotate at all)
	half2 theta12_cs = normalize(vcDir.xy);
	// compute sine and cosine of phi angle
	half2 phi12_cs;
	phi12_cs.x = sqrt(1.h - vcDir.z * vcDir.z);
	phi12_cs.y = vcDir.z;
	half4 vResult;
	// The first band is rotation-independent
	vResult.x = vZHCoeffs.x;
	// rotating the second band of SH
	vResult.y = vZHCoeffs.y * phi12_cs.x * theta12_cs.y;
	vResult.z = -vZHCoeffs.y * phi12_cs.y;
	vResult.w = vZHCoeffs.y * phi12_cs.x * theta12_cs.x;
	return vResult;
}

half4 SHProjectCone(const in half3 vcDir, uniform half angle)
{
	half2 vZHCoeffs = half2(
		.5h * (1.h - cos(angle)), // 1/2 (1 - Cos[\[Alpha]])
		0.75h * sin(angle) * sin(angle)); // 3/4 Sin[\[Alpha]]^2
	return SHRotate(vcDir, vZHCoeffs);
}

half4 SHProjectCone(const in half3 vcDir)
{
	static const half2 vZHCoeffs = half2(.25h, // 1/4
		.5h); // 1/2
	return SHRotate(vcDir, vZHCoeffs);
}

#endif