#version 330 core

in vec2 TexCoords;
flat in int index;

out vec4 color;

uniform sampler2D image;
uniform sampler2D gradient1;
uniform sampler2D gradient2;
uniform vec4 spriteColor[200];
uniform vec4 sourceRect[200];
uniform bool flipX[200], flipY[200];
uniform float time;

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

	float rawBubble = texture(image, uv).r;

	float grad1 = texture(gradient1, TexCoords.yx + vec2(time * 0.25, 0)).r;
	float grad2 = texture(gradient2, TexCoords.yx + vec2(time * 0.50, 0)).r;
	float grad = mix(grad1, grad2, 0.5);
	//float grad = abs(sin((uv.y + time) * 4.0));

	float edge = min(abs(sin(time * 6.0) * 0.8) * grad, 1.0);
	float cutOff = smoothstep(edge, 0.2 + edge, rawBubble);

	color = vec4(spriteColor[index].rgb, cutOff * spriteColor[index].a);

	//debug sliver of gradient
	//if (uv.x < 0.05) color = vec4(grad, grad, grad, 1.0);
}
