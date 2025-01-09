in vec2 TexCoords;
flat in int index;

out vec4 fragColor;

#include "common.fs"

layout(binding=0) uniform sampler2D image;
uniform vec4 spriteColor[200];
uniform vec4 sourceRect[200];
uniform bool flipX[200], flipY[200];

void main()
{
	//Determine pixel size in 0-1
	vec2 ps = 1.0 / ScreenRes.xy;

	fragColor = texture(image, TexCoords);

/*
	//OUTLINE
	//-------
	vec3 c = fragColor.rgb;
	vec3 n = texture(image, TexCoords - vec2(ps.x, 0)).rgb;
	vec3 s = texture(image, TexCoords + vec2(ps.x, 0)).rgb;
	vec3 e = texture(image, TexCoords - vec2(0, ps.y)).rgb;
	vec3 w = texture(image, TexCoords + vec2(0, ps.y)).rgb;
	vec3 edge = max(abs(n - s), abs(e - w)) * 4.0;
	float bri = 1.0 - dot(edge, vec3(0.299, 0.587, 0.114));
	fragColor = vec4(c * bri, 1.0);
*/

/*
	//SCANLINES
	//---------
	fragColor = fragColor * mod(floor(TexCoords.y * ScreenRes.y), 2.0);
*/

/*
	//CHROMATIC ABBERATION
	//--------------------
	float r = texture(image, TexCoords + vec2(ps.x * 4.0, 0)).r;
	float g = fragColor.g;
	float b = texture(image, TexCoords + vec2(-ps.x * 4.0, 0)).b;

	fragColor = vec4(r, g, b, 1.0);
*/

/*
	//PIXELATE
	//--------
	const vec2 pixelSize = vec2(320.0, 240.0);
	fragColor = texture(image, floor(TexCoords * pixelSize) / pixelSize);
*/

/*
	//FILMGRAIN
	//---------
	float strength = 16.0;
	float x = (TexCoords.x + 4.0 ) * (TexCoords.y + 4.0 ) * (TotalTime * 10.0);
	vec4 grain = vec4(mod((mod(x, 13.0) + 1.0) * (mod(x, 123.0) + 1.0), 0.01)-0.005) * strength;
	//grain = 1.0 - grain;
	//fragColor *= grain;
	fragColor += grain;
*/
}
