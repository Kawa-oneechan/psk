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


	vec4 albedo = texture(albedoTexture, vec3(uv, layer));
	vec3 normal = texture(normalTexture, vec3(uv, layer)).rgb;
	vec4 mixx = texture(mixTexture, vec3(uv, layer));
	vec4 opacity = texture(opacityTexture, vec3(uv, layer));

	if (layer == 0)
	{
		//Special grass mode. Opacity will contain color map.
		albedo.rgb = texture(opacityTexture, vec3(mixx.a, GrassColor, 0)).rgb;
	}

	if (mixx.r == mixx.g && mixx.g == mixx.b)
	{
		mixx.g = mixx.b = 0;
	}

	vec3 norm = Toon ? normalize(Normal) : calcNormal(normal);

	vec3 viewDir = normalize(viewPos - FragPos);

	vec3 result;
	for (int i = 0; i < NUMLIGHTS; i++)
		result += getLight(Lights[i], albedo.rgb, norm, viewDir, mixx.b);
	fragColor = vec4(result, 1.0);

	//fragColor = texture(albedoTexture, TexCoord);
	//fragColor = vec4(norm, 1.0);
	//fragColor = mixx;
	if(fragColor.a < 0.1) discard;
}
