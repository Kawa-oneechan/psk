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

vec4 lookup(vec2 coords)
{
	return lookup(texture(image, coords), colorTexture);
}

void main()
{
	if (PostEffect == 1)
	{
		//SCANLINES
		//---------
		fragColor = lookup(TexCoords);
		fragColor = fragColor * mod(floor(TexCoords.y * ScreenRes.y), 2.0);
	}
	else if (PostEffect == 2)
	{
		//CHROMATIC ABBERATION
		//--------------------
		vec2 ps = 1.0 / ScreenRes.xy;
		float r = lookup(TexCoords + vec2(ps.x * 4.0, 0)).r;
		float g = lookup(TexCoords).g;
		float b = lookup(TexCoords + vec2(-ps.x * 4.0, 0)).b;
		fragColor = vec4(r, g, b, 1.0);
	}
	else if (PostEffect == 3)
	{
		//PIXELATE
		//--------
		const vec2 pixelSize = vec2(320.0, 240.0);
		fragColor = lookup(floor(TexCoords * pixelSize) / pixelSize);
	}
	else if (PostEffect == 4)
	{
		//FILMGRAIN
		//---------
		float strength = 16.0;
		float x = (TexCoords.x + 4.0 ) * (TexCoords.y + 4.0 ) * (TotalTime * 10.0);
		vec4 grain = vec4(mod((mod(x, 13.0) + 1.0) * (mod(x, 123.0) + 1.0), 0.01)-0.005) * strength;
		//grain = 1.0 - grain;
		//fragColor = lookup(TexCoords) * grain;
		fragColor = lookup(TexCoords) + grain;
	}
	
	else
	{
		fragColor = lookup(TexCoords);
	}
}
