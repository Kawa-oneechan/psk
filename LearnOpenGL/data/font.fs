#version 330 core

in vec2 TexCoords;
flat in int index;

out vec4 color;

uniform sampler2D image;
uniform vec4 spriteColor[200];
uniform vec4 sourceRect[200];

void main()
{
	vec2 uv = TexCoords;
	vec4 sr = sourceRect[index];

	if (sr.z != 0)
	{
		uv *= sr.zw;
		uv += sr.xy;
	}

	color.rgb = spriteColor[index].rgb;
	color.a = texture(image, uv).r * spriteColor[index].a;
}
