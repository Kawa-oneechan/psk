#version 330 core
//#define TOON

out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 Tangent;
in vec3 FragPos;

uniform sampler2DArray albedoTexture;
uniform sampler2DArray normalTexture;
uniform sampler2DArray mixTexture;
uniform sampler2DArray opacityTexture;

uniform vec3 viewPos;

uniform int layer;

#include "lighting.fs"

void main()
{
	vec4 albedo = texture(albedoTexture, vec3(TexCoord, layer));
	vec3 normal = texture(normalTexture, vec3(TexCoord, layer)).rgb;
	vec4 mix = texture(mixTexture, vec3(TexCoord, layer));
	vec4 opacity = texture(opacityTexture, vec3(TexCoord, layer));

	//normal = normal * 2.0 - 1.0;
#ifdef TOON
	vec3 norm = normalize(Normal);
#else
	vec3 norm = calcNormal(normal);
#endif

	//vec3 ambient = ambientLight(lightColor);
	vec3 viewDir = normalize(viewPos - FragPos);

	//vec3 diffuse = diffuseLight(norm, lightPos, lightColor);
	//vec3 specular = vec3(0); //specularLight(norm, viewDir, lightPos, lightColor, mix.g);
	//vec3 result = (ambient + diffuse + specular) * albedo.rgb;
	vec3 result;
	for (int i = 0; i < NUMLIGHTS; i++)
		result += getLight(lights[i], albedo.rgb, norm, viewDir, 0.2);
	FragColor = vec4(result, opacity.r);

	//FragColor = texture(albedoTexture, TexCoord);
	//FragColor = vec4(norm, 1.0);
	//FragColor = mix;
	if(FragColor.a < 0.1) discard;
}
