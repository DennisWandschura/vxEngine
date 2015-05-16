#version 450

#include "uniform_buffers.glsl"

U_CAMERABLOCK;

layout(location = 0) in vec3 inputPosition;
layout(location = 1) in vec3 inputColor;

out VS_OUT
{
	flat vec3 color;
} vsout;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	vec4 wsPosition = vec4(inputPosition, 1.0);
	vsout.color = inputColor;
	gl_Position = u_camera.pvMatrix * wsPosition;
}