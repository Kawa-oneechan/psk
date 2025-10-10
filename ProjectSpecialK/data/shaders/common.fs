//---Common---

#define OPACITY_CUTOFF 0.5

#define NUMLIGHTS 8

struct light {
	vec4 pos;
	vec4 color;
};

layout (std140) uniform CommonData
{
	float TotalTime;
	float DeltaTime;
	uvec2 ScreenRes;
	mat4 View;
	mat4 Projection;
	mat4 InvView;
	light Lights[NUMLIGHTS];
	int PostEffect;
	float CurveAmount;
	float CurvePower;
	bool Curve;
	bool Toon;
	float GrassColor;
	float TimeOfDay;
	float HorizonPitch;
	vec4 PlayerSkin;
	vec4 PlayerEyes;
	vec4 PlayerCheeks;
	vec4 PlayerHair;
	vec4 PlayerHairHi;
};

//------------
