#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in ivec4 aBones; 
layout (location = 5) in vec4 aWeights;

//Match these to Model.h
const int MAX_BONES = 50;
const int MAX_WEIGHTS = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;
out vec3 Tangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	vec4 totalPosition = vec4(0.0);
	for(int i = 0; i < MAX_WEIGHTS; i++)
	{
		if (aBones[i] == -1) continue;
		vec4 localPosition = finalBonesMatrices[aBones[i]] * vec4(aPos, 1.0f);
		totalPosition += localPosition * aWeights[i];
	}

	FragPos = vec3(model * vec4(aPos, 1.0));
	Normal = mat3(transpose(inverse(model))) * aNormal;
	Tangent = mat3(transpose(inverse(model))) * aTangent;

	gl_Position = projection * view * model * totalPosition;
	TexCoord = aTexCoord;
}
