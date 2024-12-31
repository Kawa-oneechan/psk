out vec4 FragColor;
in vec2 TexCoord;
in vec3 Normal;
in vec3 Tangent;
in vec3 FragPos;

uniform sampler2DArray albedoTexture;
uniform sampler2DArray normalTexture;
uniform sampler2DArray mixTexture;

uniform vec3 viewPos;

uniform vec3 skinColor, eyeColor;
uniform int layer;
uniform int part;

#include "lighting.fs"

void main( out vec4 fragColor, in vec2 fragCoord )
{
/*
	alb rgb = rgb
	alb a	= sclera to pupil
	mix r	= skin to eyes
	mix g	= nothing?
	mix b	= eye brightness tweak?
			  top left is 1 for mouths and skin, 0 for eyes, ~0.38 for nose
			  for mouths, mix alb rgb directly to skin.
			  for nose, don't mix alb rgb at all.
	mix a	= ?
*/
	vec4 albedo = texture(albedoTexture, vec3(TexCoord, layer));
	vec4 mixx = texture(mixTexture, vec3(TexCoord, layer));

	vec3 finalbedo = albedo;

	if (part == 1) //eyes, needs pupil color
		finalbedo = mix(mix(albedo.rgb, eyeColor, albedo.a) + mixx.b, skinColor, mixx.r);
	else if (part == 2) //mouth
		finalbedo = mix(albedo.rgb, skinColor, mixx.r);
	else if (part == 3) //cheeks (use eye color for blush)
		finalbedo = mix(eyeColor, skinColor, albedo.r);
	else if (part == 4) //socks
		finalbedo = mix(skinColor, albedo.rgb, albedo.a); //not sure if this is the right order

	vec3 normal = texture(normalTexture, vec3(TexCoord, layer)).rgb;

	vec3 norm = Toon ? normalize(Normal) : calcNormal(normal);

	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 result;
	for (int i = 0; i < NUMLIGHTS; i++)
		result += getLight(lights[i], finalbedo, norm, viewDir, mix.b);
	FragColor = vec4(result, opacity.r);	
}
