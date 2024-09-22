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

#include "lighting.fs"

void main( out vec4 fragColor, in vec2 fragCoord )
{
/*
	alb rgb = rgb
	alb a   = sclera to pupil
	mix r   = skin to eyes
	mix g   = nothing?
	mix b   = eye brightness tweak?
			  top left is 1 for mouths and skin, 0 for eyes, ~0.38 for nose
			  for mouths, mix alb rgb directly to skin.
			  for nose, don't mix alb rgb at all.
	mix a   = ?
*/
	vec4 albedo = texture(albedoTexture, vec3(TexCoord, layer));
	vec4 mixx = texture(mixTexture, vec3(TexCoord, layer));

	vec3 finalbedo;
	
	float mxb = texture(ch1, vec2(0.1)).b;
	if (mxb < 0.3) //eyes, needs pupil color
		finalbedo = mix(mix(albedo.rgb, eyeColor, albedo.a) + mixx.b, skinColor, mixx.r);
	else if (mxb > 0.7) //mouth
		finalbedo = mix(albedo.rgb, skinColor, mixx.r);
   else //nose
		texture(ch1, vec2(0.1)).b


	vec3 normal = texture(normalTexture, vec3(TexCoord, layer)).rgb;

#ifdef TOON
	vec3 norm = normalize(Normal);
#else
	vec3 norm = calcNormal(normal);
#endif

	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 result;
	for (int i = 0; i < NUMLIGHTS; i++)
		result += getLight(lights[i], finalbedo, norm, viewDir, mix.b);
	FragColor = vec4(result, opacity.r);	
}