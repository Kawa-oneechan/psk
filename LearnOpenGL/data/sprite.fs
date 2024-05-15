#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;
uniform vec4 spriteColor;
uniform vec4 sourceRect;
uniform bool flipX, flipY;

void main()
{
	vec2 uv = TexCoords;
	vec4 sr = sourceRect;

	if (sourceRect.z != 0)
	{
		uv.x -= (uv.x * 2) * float(flipX);
		sr.x += sr.w * float(flipX);

		uv.y -= (uv.y * 2) * float(flipY);
		sr.y -= sr.z * float(flipY);

		uv *= sr.zw;
		uv += sr.xy;
	}

	color = vec4(spriteColor.rgb, 1.0) * texture(image, uv);
	color.a *= spriteColor.a;
}
