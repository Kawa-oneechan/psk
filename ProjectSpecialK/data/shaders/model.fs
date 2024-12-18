//#define TOON

in vec2 TexCoord;
in vec3 Normal;
in vec3 Tangent;
in vec3 FragPos;

out vec4 fragColor;

layout(binding=0) uniform sampler2DArray albedoTexture;
layout(binding=1) uniform sampler2DArray normalTexture;
layout(binding=2) uniform sampler2DArray mixTexture;
layout(binding=3) uniform sampler2DArray opacityTexture;

uniform vec3 viewPos;
uniform int layer;

#include "common.fs"
#include "lighting.fs"

void main()
{
	vec4 albedo = texture(albedoTexture, vec3(TexCoord, layer));
	vec3 normal = texture(normalTexture, vec3(TexCoord, layer)).rgb;
	vec4 mixx = texture(mixTexture, vec3(TexCoord, layer));
	vec4 opacity = texture(opacityTexture, vec3(TexCoord, layer));

	if (mixx.r == mixx.g && mixx.g == mixx.b)
	{
		mixx.g = mixx.b = 0;
	}

	//normal = normal * 2.0 - 1.0;
#ifdef TOON
	vec3 norm = normalize(Normal);
#else
	vec3 norm = calcNormal(normal);
#endif

	vec3 viewDir = normalize(viewPos - FragPos);

	vec3 result;
	for (int i = 0; i < NUMLIGHTS; i++)
		result += getLight(Lights[i], albedo.rgb, norm, viewDir, mixx.b);
	fragColor = vec4(result, opacity.r);

	//fragColor = texture(albedoTexture, TexCoord);
	//fragColor = vec4(norm, 1.0);
	//fragColor = mixx;
	if(fragColor.a < 0.1) discard;
}
