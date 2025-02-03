in vec2 TexCoords;
flat in int index;

out vec4 fragColor;

#include "common.fs"

layout(binding=0) uniform sampler2D image;
uniform vec4 spriteColor[200];
uniform vec4 sourceRect[200];
uniform bool flipX[200], flipY[200];

layout(binding=1) uniform sampler2D colorTexture;

vec4 lookup(in vec4 textureColor, in sampler2D lookupTable) {
	mediump float blueColor = textureColor.b * 63.0;

	mediump vec2 quad1;
	quad1.y = floor(floor(blueColor) / 8.0);
	quad1.x = floor(blueColor) - (quad1.y * 8.0);

	mediump vec2 quad2;
	quad2.y = floor(ceil(blueColor) / 8.0);
	quad2.x = ceil(blueColor) - (quad2.y * 8.0);

	highp vec2 texPos1;
	texPos1.x = (quad1.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.r);
	texPos1.y = (quad1.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.g);

	texPos1.y = 1.0-texPos1.y;

	highp vec2 texPos2;
	texPos2.x = (quad2.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.r);
	texPos2.y = (quad2.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.g);

	texPos2.y = 1.0-texPos2.y;

	lowp vec4 newColor1 = texture2D(lookupTable, texPos1);
	lowp vec4 newColor2 = texture2D(lookupTable, texPos2);

	lowp vec4 newColor = mix(newColor1, newColor2, fract(blueColor));
	return newColor;
}

void main()
{
	fragColor = lookup(texture(image, TexCoords), colorTexture);
}
