in vec2 TexCoord;
in vec3 Normal;
in vec3 Tangent;
in vec3 FragPos;

out vec4 fragColor;

layout(binding=3) uniform sampler2DArray opacityTexture;

uniform vec3 viewPos;
uniform int layer;

#include "common.fs"

void main()
{
	if (texture(opacityTexture, vec3(TexCoord, layer)).r < 0.1)
		discard;
	fragColor = vec4(0.0);
}
