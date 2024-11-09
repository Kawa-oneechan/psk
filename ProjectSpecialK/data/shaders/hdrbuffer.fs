#version 330 core

in vec2 TexCoords;
flat in int index;

out vec4 color;

#include "common.txt"

uniform sampler2D image;

void main()
{
	vec3 hdrColor = texture(image, TexCoords).rgb;
	
	const float gamma = 0.4;
	const float exposure = 3.0;

	vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
	mapped = pow(mapped, vec3(1.0 / gamma));

	color = vec4(mapped, 1.0);
}
