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
	float opacity = texture(albedoTexture, uv3).r;
	vec3 normal = texture(normalTexture, uv3).rgb;
	vec4 mixx = texture(mixTexture, uv3);

	if (mixx.r == mixx.g && mixx.g == mixx.b)
	{
		mixx.g = mixx.b = 0;
	}

	vec3 norm = Toon ? normalize(Normal) : calcNormal(normal);

	vec3 viewDir = normalize(viewPos - FragPos);

	vec3 albedo = texture(opacityTexture, vec3(mixx.a, GrassColor, 0)).rgb;

	vec3 result;
	for (int i = 0; i < NUMLIGHTS; i++)
		result += getLight(Lights[i], albedo, norm, viewDir, mixx.b);

	fragColor = vec4(result, opacity);

	//fragColor = texture(albedoTexture, TexCoord);
	//fragColor = vec4(norm, 1.0);
	//fragColor = mixx;
	if(fragColor.a < 0.1) discard;
}
