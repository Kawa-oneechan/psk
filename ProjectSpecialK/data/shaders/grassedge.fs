in vec2 TexCoord;
in vec3 Normal;
in vec3 Tangent;
in vec3 FragPos;

out vec4 fragColor;

layout(binding=0) uniform sampler2DArray albedoTexture; //now opacity
layout(binding=1) uniform sampler2DArray normalTexture;
layout(binding=2) uniform sampler2DArray mixTexture;
layout(binding=3) uniform sampler2DArray opacityTexture; //now colormap

uniform vec3 viewPos;
uniform int layer;
uniform mat4 model;

#include "common.fs"
#include "lighting.fs"

void main()
{
	vec3 uv3 = vec3(TexCoord, layer);
	float opacityVal = texture(albedoTexture, uv3).r;
	vec3 normalVal = texture(normalTexture, uv3).rgb;
	vec4 mixVal = texture(mixTexture, uv3);

	if (mixVal.r == mixVal.g && mixVal.g == mixVal.b)
	{
		mixVal.g = mixVal.b = 0;
	}

	vec3 norm = Toon ? normalize(Normal) : calcNormal(normalVal);

	vec3 viewDir = normalize(viewPos - FragPos);

	vec3 albedoVal = texture(opacityTexture, vec3(mixVal.a, GrassColor, 0)).rgb;

	vec3 result;
	for (int i = 0; i < NUMLIGHTS; i++)
		result += getLight(Lights[i], albedoVal, norm, viewDir, mixVal.b);

	fragColor = vec4(result, opacityVal);

	if(fragColor.a < OPACITY_CUTOFF) discard;
}
