//---Common---

#define NUMLIGHTS 8

struct light {
	vec4 pos;
	vec4 color;
};

layout (std140) uniform CommonData
{
	float TotalTime; //0
	float DeltaTime; //4
	float CurveAmount; //8
	float CurvePower; //12
	bool Curve; //16
	bool Toon; //20
	uvec2 ScreenRes; //24
	mat4 View; //32
	mat4 Projection; //96
	mat4 InvView; //160
	light Lights[NUMLIGHTS]; //224
};

//------------
