#version 330 core
in vec2 TexCoords;
out vec4 color;

//common.txt
layout (std140) uniform CommonData
{
	float totalTime; //0
	float deltaTime; //4
	uvec2 screenRes; //8
	mat4 View; //16
	mat4 Projection; //80
};

uniform float pitch;

void main()
{
	vec2 uv = gl_FragCoord.xy / screenRes.xy;
	uv.y = -uv.y + 1.0;

	#define SKY_TOP vec3(0.47, 0.56, 0.68)
	#define SKY_MID vec3(0.63, 0.72, 0.80)
	#define SKY_BOT vec3(0.92, 0.89, 0.78)
	#define GND_TOP vec3(0.60, 0.63, 0.67)
	#define GND_BOT vec3(0.00, 0.00, 0.00)
    
	//#define HORIZON (-(iMouse.y / screenRes.y) + 1.0)
	//#define HORIZON .50
	//#define HORIZON (((pitch + 90) / 90.0) * 0.1)
	float horizon = 0.5;
	if (pitch > 0)
		horizon -= (0.5 * (pitch / 60));
	else
		horizon -= (0.5 * (pitch / 30));
    
	vec3 c = vec3(0);
	vec3 e = mix(SKY_BOT, SKY_TOP, horizon);
    
	if (uv.y < horizon)
		c = mix(SKY_TOP, e, uv.y * (1.0 / horizon));
	else
		c = mix(e, GND_BOT, (uv.y - horizon) * 10.0);

	color = vec4(c, 1.0);
}
