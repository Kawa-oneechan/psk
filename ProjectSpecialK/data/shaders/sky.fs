//Edited from shadertoy Ntd3Ws

in vec2 TexCoords;

out vec4 fragColor;

#include "common.fs"

layout(binding=1) uniform sampler2DArray cloudImage;
layout(binding=2) uniform sampler2D starsImage;
layout(binding=3) uniform sampler2D skyImage;

float clouds(vec2 uv, float pitch)
{
	float p = pitch; //0.25 + (HorizonPitch / 90.0);
	float uvy = uv.y; //for later

	//based on Shadertoy XfXXz2 by darkomtc
	vec3 viewDir = vec3(uv.x - 0.5, uv.y - p, 0.5);
	vec2 planarUV = viewDir.xz / abs(viewDir.y);

	uv = planarUV * 0.24;
	uv.y += 0.5;
	uv.x *= 0.4;
	uv += TotalTime * 0.045;

	float n1 = texture(cloudImage, vec3(uv * 0.25, 0)).r * 0.5;
	float n2 = texture(cloudImage, vec3(uv + vec2(0.5) * 0.15, 0)).r * 0.5;
	float ret = n1 + n2;

	float fade = smoothstep(p, p + 0.5, uvy);
	ret *= fade;

	return ret;
}


void main()
{
	vec2 uv = gl_FragCoord.xy / ScreenRes.xy;
	float pit = HorizonPitch / 90.0;

	//vec3 viewPos = (InvView * vec4(0.0, 0.0, 0.0, 0.1)).xyz;
	//vec3 viewDir = normalize(viewPos); // gl_FragCoord.xyz);
	//float pit = 0.5 + viewPos.g * 0.025;
	//uv.x -= (viewPos.r * 0.001);

	vec3 sky = texture(skyImage, vec2(TimeOfDay, uv.y - 0.01)).rgb;
	fragColor = vec4(sky, 1.0);

	float blend = clamp(texture(skyImage, vec2(TimeOfDay, 0.0)).r * 3.0, 0.15, 1.0);

	vec4 starsColor = vec4(
			texture(starsImage, uv + vec2(0, -pit)).rgb +
			mix(NightSkyColor, vec3(0), uv.y),
		1.0);

	fragColor = mix(starsColor, fragColor, blend);
	fragColor = mix(fragColor, vec4(1), clouds(uv * 1.25, pit) * (blend * 0.75));

	if (uv.y < pit)
		fragColor = vec4(0.0, 0.0, 0.0, 1.0);

	//fragColor.rgb = viewPos;
}
