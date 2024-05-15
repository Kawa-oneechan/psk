#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;
uniform vec4 spriteColor;
uniform vec4 sourceRect;

void main()
{
	vec2 uv = TexCoords;
	vec4 sr = sourceRect;

	if (sourceRect.z != 0)
	{
		uv *= sr.zw;
		uv += sr.xy;
	}

	color.rgb = spriteColor.rgb;
	color.a = texture(image, uv).r * spriteColor.a;
}
