//Edited from shadertoy Ntd3Ws

in vec2 TexCoords;

out vec4 fragColor;

#include "common.fs"

layout(binding=1) uniform sampler2D cloudImage;
layout(binding=2) uniform sampler2D starsImage;
layout(binding=3) uniform sampler2D skyImage;

//TODO: find out how to derive this from the camera view matrix
uniform float pitch;

/*
float clouds(vec2 uv)
{
	uv.y -= pitch * 0.025;

	uv.y -= 0.25;
	uv.x += TotalTime * 0.0050;
	float ret = texture(cloudImage, uv).a;
	uv.y += 0.5;
	uv.x *= 0.9;
	uv.x += TotalTime * 0.0045;
	ret += texture(cloudImage, uv * 1.7).a * 0.25;
	return ret;
}
*/

float clouds(vec2 uv)
{
	float p = 0.25 + (pitch / 90.0);
	float uvy = uv.y; //for later

	//based on Shadertoy XfXXz2 by darkomtc
	vec3 viewDir = vec3(uv.x - 0.5, uv.y - p, 0.5);
	vec2 planarUV = viewDir.xz / abs(viewDir.y);

	uv = planarUV * 0.25;
	uv.y -= 0.25;
	uv += TotalTime * 0.050;
	float ret = texture(cloudImage, uv).a;

	uv = planarUV * 0.26;
	uv.y += 0.5;
	uv.x *= 0.4;
	uv += TotalTime * 0.045;
	ret += texture(cloudImage, uv * 1.7).a * 0.25;

	uv = planarUV * 0.24;
	uv.y += 0.5;
	uv.x *= 0.4;
	uv += TotalTime * 0.045;
	ret += texture(cloudImage, uv * 1.7).a * 0.35;

    float fade = smoothstep(p, p + 0.5, uvy);
    ret *= fade;

	return ret;
}


void main()
{
	vec2 uv = gl_FragCoord.xy / ScreenRes.xy;
	float pit = pitch / 90.0;

	vec3 sky = texture(skyImage, vec2(TimeOfDay, uv.y - 0.01)).rgb;
	fragColor = vec4(sky, 1.0);

	float blend = clamp(texture(skyImage, vec2(TimeOfDay, 0.0)).r * 3.0, 0.0, 1.0);

	fragColor = mix(texture(starsImage, uv), fragColor, blend);
	fragColor = mix(fragColor, vec4(1), clouds(uv * 1.25) * (blend * 0.75));

	if (uv.y < pit)
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
