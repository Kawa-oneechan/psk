#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;
uniform vec4 sourceRect;
uniform float time;

void main()
{
	vec2 uv = TexCoords;
	vec4 sr = sourceRect;
	uv *= sr.zw;
	uv += sr.xy;
	color = texture(image, uv + (vec2(time, -time) * 0.1));
}
