float getFalloff(float distance, float lightFalloff)
{
	float result = clamp(1.0 - pow(distance / lightFalloff, 4.0), 0.0, 1.0);
	return result * result / (distance * distance + 1.0);
}