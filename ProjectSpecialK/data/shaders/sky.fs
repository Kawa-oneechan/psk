#version 330 core
in vec2 TexCoords;
out vec4 color;

#include "common.txt"

uniform sampler2D cloudImage;
uniform sampler2D starsImage;

uniform float pitch;

vec3 stars(vec2 uv)
{
	return texture(starsImage, uv * 2.0).rgb;
}

float clouds(vec2 uv)
{
	uv.y -= 0.25;
	uv.x += totalTime * 0.00005;
	float ret = texture(cloudImage, uv).a;
	uv.y += 0.5;
	uv.x *= 0.9;
	uv.x += totalTime * 0.000045;
	ret += texture(cloudImage, uv * 1.7).a * 0.25;
	return ret;
}

void main()
{
	vec2 uv = gl_FragCoord.xy / screenRes.xy;
	float time = 1.0; //abs(sin(totalTime * 0.000025));

	float flipped = -uv.y + 1.0;

	#define SKY_TOP vec3(0.47, 0.56, 0.68)
	#define SKY_MID vec3(0.63, 0.72, 0.80)
	#define SKY_BOT vec3(0.92, 0.89, 0.78)
	#define GND_TOP vec3(0.60, 0.63, 0.67)
	#define GND_BOT vec3(0.00, 0.00, 0.00)

	float horizon = 0.5;
	if (pitch > 0.0)
		horizon -= (0.5 * (pitch / 60.0));
	else
		horizon -= (0.5 * (pitch / 30.0));

	vec3 c = vec3(0);
	vec3 e = mix(SKY_MID, SKY_TOP, horizon);
	e = mix(e, vec3(1.0), clouds(uv + vec2(0.0, horizon + 0.25)));
	
	if (flipped < horizon)
	{
		c = mix(e, SKY_TOP, flipped * (1.0 / horizon));
		c = mix(stars(uv), c, time); //TODO: needs better easing
	}

	color = vec4(c, 1.0);
}
