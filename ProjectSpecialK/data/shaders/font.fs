in vec2 TexCoords;
flat in int index;

out vec4 fragColor;

layout(binding=0) uniform sampler2D image;
uniform vec4 spriteColor[200];
uniform vec4 sourceRect[200];

void main()
{
	vec2 uv = TexCoords;
	vec4 sr = sourceRect[index];

	sr.y = -sr.y;
	sr.w = -sr.w;

	if (sr.z != 0)
	{
		uv *= sr.zw;
		uv += sr.xy;
	}

	fragColor.rgb = spriteColor[index].rgb;
	fragColor.a = texture(image, uv).r * spriteColor[index].a;
}
