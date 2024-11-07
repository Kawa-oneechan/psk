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
	//vec4 lol = texture(image, TexCoords);
	//color = vec4(1.0 - lol.rgb, lol.a);
	color = texture(image, TexCoords);

	float y = floor(TexCoords.y * 1080);
	if (mod(y, 2.0) == 0.0)
		color.a = 0.0;
}
