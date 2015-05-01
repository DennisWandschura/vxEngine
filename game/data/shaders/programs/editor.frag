#version 450
#extension GL_ARB_bindless_texture : require

#define U16_MAX 0xffff

#include "uniform_buffers.glsl"
#include "buffers.glsl"

B_MATERIALBLOCK;
B_TEXTUREBLOCK;

in VS_OUT
{
	vec3 wsPosition;
	vec3 vsPosition;
	vec3 normal;
	vec3 tangent;
	vec3 bitangent;
	vec2 texCoords;
	flat uint materialId;
} input;

layout(location = 0) out vec4 fragColor;

void main()
{
	Material material = b_materials[input.materialId];
	uint albedoIndex = material.indexAlbedo & U16_MAX;
	uint albedoSlice = (material.indexAlbedo >> 16) & U16_MAX;

	// albedoSlice : rgb8
	// normalSlice : RGB16F
	// surface : rgba8

	vec4 albedo = texture(u_textures[albedoIndex], vec3(input.texCoords, albedoSlice));

	fragColor = vec4(albedo.rgb, 1.0);
}