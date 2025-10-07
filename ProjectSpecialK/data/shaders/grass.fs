in vec2 TexCoord;
in vec3 Normal;
in vec3 Tangent;
in vec3 FragPos;
in vec3 WorldPos;

out vec4 fragColor;

layout(binding=0) uniform sampler2DArray albedoTexture;
layout(binding=1) uniform sampler2DArray normalTexture;
layout(binding=2) uniform sampler2DArray mixTexture;
layout(binding=3) uniform sampler2DArray colorMapTexture;

uniform vec3 viewPos;
uniform int layer;
uniform mat4 model;

#include "common.fs"
#include "lighting.fs"

void main()
{
	float m00 = round(model[0][0]);
	float m20 = round(model[2][0]);
	int rotation = 0;
	vec2 uv = TexCoord;
	     if (m00 >=  1.0 && m20 ==  0.0) rotation = 0;
	else if (m00 ==  0.0 && m20 <= -1.0) rotation = 1;
	else if (m00 <= -1.0 && m20 ==  0.0) rotation = 2;
	else if (m00 ==  0.0 && m20 >=  1.0) rotation = 3;
    for (int i = 0; i < rotation; i++)
    {
        uv.xy = uv.yx;
        uv.x = 1.0 - uv.x;
    }
	if (rotation == 3)
		uv.xy = 1.0 - uv.xy;

	vec4 albedoVal = texture(albedoTexture, vec3(uv, layer));
	vec3 normalVal = texture(normalTexture, vec3(uv, layer)).rgb;
	vec4 mixVal = texture(mixTexture, vec3(uv, layer));

	if (layer == 0) //regular grass or snow, no design
		albedoVal.rgb = texture(colorMapTexture, vec3(mixVal.a, GrassColor, 0)).rgb;

	if (mixVal.r == mixVal.g && mixVal.g == mixVal.b)
		mixVal.g = mixVal.b = 0;

	float specularVal = mixVal.b;
	const float opacityVal = 1.0;
	const float fresnelVal = 0.0;

#include "model_generic_bottom.fs"
