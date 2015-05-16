#version 450

in VS_OUT
{
	flat vec3 color;
} input;

layout(location = 0) out vec4 fragColor;

void main()
{
	fragColor = vec4(input.color, 0.5f);
}