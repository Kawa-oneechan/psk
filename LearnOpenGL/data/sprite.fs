#version 330 core

in vec2 TexCoords;
flat in int index;

out vec4 color;

uniform sampler2D image;
uniform vec4 spriteColor[200];
uniform vec4 sourceRect[200];
uniform bool flipX[200], flipY[200];

void main()
{
	vec2 uv = TexCoords;
	vec4 sr = sourceRect[index];

	if (sr.z != 0)
	{
		uv.x -= (uv.x * 2) * float(flipX[index]);
		sr.x += sr.w * float(flipX[index]);

		uv.y -= (uv.y * 2) * float(flipY[index]);
		sr.y -= sr.z * float(flipY[index]);

		uv *= sr.zw;
		uv += sr.xy;
	}

	color = vec4(spriteColor[index].rgb, 1.0) * texture(image, uv);
	color.a *= spriteColor[index].a;
}
