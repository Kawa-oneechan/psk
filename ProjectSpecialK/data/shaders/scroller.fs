in vec2 TexCoords;

out vec4 fragColor;

layout(binding=0) uniform sampler2D image;
uniform vec4 sourceRect;
uniform float time;
uniform vec2 speed;
uniform vec4 recolorB;
uniform vec4 recolorW;

void main()
{
	vec2 uv = gl_FragCoord.xy / vec2(1920, 1080); //normalized frag coords instead of the TexCoords from the vertex shader;
	vec4 sr = sourceRect;
	uv *= sr.zw;
	uv += sr.xy;
	fragColor = texture(image, uv + vec2(time * speed.x, time * speed.y));

	if (recolorB.a != 0.0)
		fragColor = mix(recolorB, recolorW, fragColor.r);
}
