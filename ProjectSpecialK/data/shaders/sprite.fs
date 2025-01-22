in vec2 TexCoords;
flat in int index;

out vec4 fragColor;

//Match this to SpriteRenderer.h
const int BatchSize = 200;

layout(binding=0) uniform sampler2D image;
uniform vec4 spriteColor[BatchSize];
uniform vec4 sourceRect[BatchSize];
uniform bool flipX[BatchSize], flipY[BatchSize];

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

	fragColor = vec4(spriteColor[index].rgb, 1.0) * texture(image, uv);
	fragColor.a *= spriteColor[index].a;
}
