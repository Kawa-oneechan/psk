in vec2 TexCoords;
flat in int index;

out vec4 fragColor;

#include "common.fs"

uniform sampler2D image;
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

    float r = texture(image, TexCoords + vec2(pixSize, 0)).r;
    float g = texture(image, TexCoords).g;
    float b = texture(image, TexCoords + vec2(-pixSize, 0)).b;

    fragColor = vec4(r, g, b, 1.0);
}
