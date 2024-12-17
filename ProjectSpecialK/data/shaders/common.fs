//---Common---

layout (std140) uniform CommonData
{
	float totalTime; //0
	float deltaTime; //4
	uvec2 screenRes; //8
	mat4 View; //16
	mat4 Projection; //80
	mat4 InvView; //144
};

//------------
