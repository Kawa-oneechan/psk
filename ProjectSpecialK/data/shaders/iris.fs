in vec2 TexCoords;
flat in int index;

out vec4 fragColor;

#include "common.fs"

uniform float smoothness;
uniform float progress;

const vec2 center = vec2(0.5, 0.5);
const float SQRT_2 = 1.414213562373;

void main()
{
	vec2 uv = gl_FragCoord.xy / screenRes.xy;

//	vec2 ratio2 = vec2(1.0, 1.0 / (screenRes.x / screenRes.y));
//	float dist = length((uv - 0.5) * ratio2);

//	float m = smoothstep(progress - smoothness, progress, dist);

	float m = smoothstep(-smoothness, 0.0, SQRT_2 * distance(center, uv) - progress * (1.0 + smoothness));

	fragColor.rgb = vec3(0.0);
	fragColor.a = mix(0.0, 1.0, m);
}
