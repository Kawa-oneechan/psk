in vec2 TexCoords;
flat in int index;

out vec4 fragColor;

#include "common.fs"

layout(binding=0) uniform sampler2D image;
uniform vec4 spriteColor[200];
uniform vec4 sourceRect[200];
uniform bool flipX[200], flipY[200];

layout(binding=1) uniform sampler2D colorTexture;

vec3 filmic(in vec3 x)
{
	vec3 X = max(vec3(0.0), x - 0.004);
	vec3 result = (X * (6.2 * X + 0.5)) / (X * (6.2 * X + 1.7) + 0.06);
	return pow(result, vec3(2.2));
}

vec3 PBRNeutralToneMapping(vec3 color)
{
	const float startCompression = 0.8 - 0.04;
	const float desaturation = 0.15;

	float x = min(color.r, min(color.g, color.b));
	float offset = x < 0.08 ? x - 6.25 * x * x : 0.04;
	color -= offset;

	float peak = max(color.r, max(color.g, color.b));
	if (peak < startCompression) return color;

	const float d = 1. - startCompression;
	float newPeak = 1. - d * d / (peak + d - startCompression);
	color *= newPeak / peak;

	float g = 1. - 1. / (desaturation * (peak - newPeak) + 1.);
	return mix(color, newPeak * vec3(1, 1, 1), g);
}

vec3 lottes(vec3 x)
{
	x *= vec3(0.9); //I reduced the light a little
	const vec3 a = vec3(1.6);
	const vec3 d = vec3(0.977);
	const vec3 hdrMax = vec3(8.0);
	const vec3 midIn = vec3(0.18);
	const vec3 midOut = vec3(0.267);

	const vec3 b =
		(-pow(midIn, a) + pow(hdrMax, a) * midOut) /
		((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);
	const vec3 c =
		(pow(hdrMax, a * d) * pow(midIn, a) - pow(hdrMax, a) * pow(midIn, a * d) * midOut) /
		((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);

	return pow(x, a) / (pow(x, a * d) * b + c);
}

vec3 lookup(in vec3 textureColor, in sampler2D lookupTable) {
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

	lowp vec3 newColor1 = texture2D(lookupTable, texPos1).rgb;
	lowp vec3 newColor2 = texture2D(lookupTable, texPos2).rgb;

	lowp vec3 newColor = mix(newColor1, newColor2, fract(blueColor));
	return newColor;
}

vec4 lookup(vec2 coords)
{
	vec3 x = texture(image, coords).rgb;
	x = PBRNeutralToneMapping(x);
	return vec4(lookup(x, colorTexture), 1.0);
}

void main()
{
	if (PostEffect == 1)
	{
		//SCANLINES
		//---------
		fragColor = lookup(TexCoords);
		fragColor.rgb *= mod(floor(TexCoords.y * ScreenRes.y), 2.0);
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
