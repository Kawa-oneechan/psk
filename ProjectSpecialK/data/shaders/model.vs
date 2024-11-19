layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in ivec4 aBones; 
layout (location = 5) in vec4 aWeights;

out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;
out vec3 Tangent;

#include "common.fs"

uniform mat4 model;

//Match these to Model.h
const int MaxBones = 50;
const int MaxWeights = 4;
uniform mat4 finalBonesMatrices[MaxBones];

void main()
{
	vec4 totalPosition = vec4(0.0);
	vec3 totalNormal = vec3(0.0);
	vec4 aPos4 = vec4(aPos, 1.0f);
	for(int i = 0; i < MaxWeights; i++)
	{
		if (aBones[i] == -1) continue;
		
		vec4 localPosition = finalBonesMatrices[aBones[i]] * aPos4;
		totalPosition += localPosition * aWeights[i];

		vec3 localNormal = mat3(finalBonesMatrices[aBones[i]]) * aNormal;
		totalNormal += localNormal * aWeights[i];
	}

	FragPos = vec3(model * totalPosition);
	Normal = mat3(transpose(inverse(model))) * totalNormal;
	Tangent = mat3(transpose(inverse(model))) * aTangent;

	gl_Position = Projection * View * model * totalPosition;
	TexCoord = aTexCoord;
}
