in vec2 TexCoords;
flat in int index;

out vec4 fragColor;

#include "common.fs"

layout(binding=0) uniform sampler2D image;
uniform vec4 spriteColor[200];
uniform vec4 sourceRect[200];
uniform bool flipX[200], flipY[200];

const float pixSize = 0.0025;

void main()
{
/*
	//vec4 lol = texture(image, TexCoords);
	//fragColor = vec4(1.0 - lol.rgb, lol.a);
	fragColor = texture(image, TexCoords);

	float y = floor(TexCoords.y * screenRes.y);
	if (mod(y, 2.0) == 0.0)
		fragColor.a = 0.0;
*/

/*
	float r = texture(image, TexCoords + vec2(pixSize, 0)).r;
	float g = texture(image, TexCoords).g;
	float b = texture(image, TexCoords + vec2(-pixSize, 0)).b;

	fragColor = vec4(r, g, b, 1.0);
*/

	const vec2 pixelSize = vec2(320.0, 240.0);
	vec2 uv = floor(TexCoords * pixelSize);
	uv /= pixelSize;
	fragColor = texture(image, uv);

/*
	vec4 color = texture(image, TexCoords);
	float strength = 16.0;
	float x = (TexCoords.x + 4.0 ) * (TexCoords.y + 4.0 ) * (totalTime * 10.0);
	vec4 grain = vec4(mod((mod(x, 13.0) + 1.0) * (mod(x, 123.0) + 1.0), 0.01)-0.005) * strength;
	//grain = 1.0 - grain;
	//fragColor = color * grain;
	fragColor = color + grain;
*/
}
