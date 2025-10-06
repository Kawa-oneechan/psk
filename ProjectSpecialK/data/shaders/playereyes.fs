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

uniform vec3 viewPos;
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

	float blendVal = mixVal.r;
	float roughnessVal = mixVal.g;
	float specularVal = mixVal.b;
	float eyeBlendVal = albedoVal.a;
	const float fresnelVal = 1.0;

	vec3 a = mix(albedoVal.rgb, PlayerEyes.rgb, eyeBlendVal);
	vec3 b = mix(a, PlayerSkin.rgb, blendVal);
	albedoVal.rgb = b;

	vec3 norm = Toon ? normalize(Normal) : calcNormal(normalVal);

	vec3 viewDir = normalize(viewPos - FragPos);

	float fresnel = getFresnel(model, norm);

	vec3 result;
	for (int i = 0; i < NUMLIGHTS; i++)
		result += getLight(Lights[i], albedoVal.rgb, norm, viewDir, specularVal) + (fresnelVal * fresnel);
	fragColor = vec4(result, opacityVal);

	if(fragColor.a < OPACITY_CUTOFF) discard;
}
