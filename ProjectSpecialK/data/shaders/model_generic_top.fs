in vec2 TexCoord;
in vec3 Normal;
in vec3 Tangent;
in vec3 FragPos;
in vec3 WorldPos;

out vec4 fragColor;

layout(binding=0) uniform sampler2DArray albedoTexture;
layout(binding=1) uniform sampler2DArray normalTexture;
layout(binding=2) uniform sampler2DArray mixTexture;
layout(binding=3) uniform sampler2DArray opacityTexture;

uniform int layer;
uniform mat4 model;

#include "common.fs"
#include "lighting.fs"

void main()
{
	vec4 albedoVal = texture(albedoTexture, vec3(TexCoord, layer));
	vec3 normalVal = texture(normalTexture, vec3(TexCoord, layer)).rgb;
	vec4 mixVal = texture(mixTexture, vec3(TexCoord, layer));
	float opacityVal = texture(opacityTexture, vec3(TexCoord, layer)).r;

	if (mixVal.r == mixVal.g && mixVal.g == mixVal.b)
		mixVal.g = mixVal.b = 0;

