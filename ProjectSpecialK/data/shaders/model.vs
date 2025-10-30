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
out vec3 WorldPos;

#include "common.fs"

uniform mat4 model;

//Match these to Model.h
const int MaxBones = 100;
const int MaxWeights = 4;
uniform mat4 finalBonesMatrices[MaxBones];

void main()
{
	bool billboard = model[0][3] > 0.0;
	mat4 _model = model;
	_model[0][3] = 0.0;

	vec4 totalPosition = vec4(0.0);
	vec3 totalNormal = vec3(0.0);
	vec4 aPos4 = vec4(aPos, 1.0f);
	for (int i = 0; i < MaxWeights; i++)
	{
		if (aBones[i] == -1) continue;

		vec4 localPosition = finalBonesMatrices[aBones[i]] * aPos4;

		totalPosition += localPosition * aWeights[i];

		vec3 localNormal = mat3(finalBonesMatrices[aBones[i]]) * aNormal;
		totalNormal += localNormal * aWeights[i];
	}

	FragPos = vec3(_model * totalPosition);
	Normal = mat3(transpose(inverse(_model))) * totalNormal;
	Tangent = mat3(transpose(inverse(_model))) * aTangent;
	WorldPos = (_model * totalPosition).xyz;

	mat4 vm = View * _model;
	if (billboard)
	{
		vm[0] = vec4(1, 0, 0, 0);
		vm[1] = vec4(0, 1, 0, 0);
		vm[2] = vec4(0, 0, 1, 0);
	}

	vec4 tm = vm * totalPosition;

	if (Curve)
	{
		vec4 delta = tm - vec4(InvView);
		tm.y += -CurveAmount * pow(abs(delta.z), CurvePower);
		tm.y += 10.0;
	}

	gl_Position = Projection * tm;
	TexCoord = aTexCoord;

}
