#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;
uniform vec4 sourceRect;
uniform float time;
uniform vec2 speed;

void main()
{
	vec2 uv = gl_FragCoord.xy / vec2(1920, 1080); //normalized frag coords instead of the TexCoords from the vertex shader;
	vec4 sr = sourceRect;
	uv *= sr.zw;
	uv += sr.xy;
	color = texture(image, uv + vec2(time * speed.x, time * speed.y));
}
