#ifdef _VX_WINDOWS
#pragma once
#endif

float getFalloff(float distance, float lightFalloff)
{
	float result = clamp(1.0 - pow(distance / lightFalloff, 4.0), 0.0, 1.0);
	return result * result / (distance * distance + 1.0);
}

float getLightIntensity(float distance, float lightFalloff, float lightLumen)
{
	float falloff = getFalloff(distance, lightFalloff);
	return falloff * lightLumen / (4 * g_PI);
}