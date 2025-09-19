#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "engine/SpriteRenderer.h"
#include "engine/TextUtils.h"
#include "engine/Utilities.h"
#include "engine/Tickable.h"
#include "engine/Audio.h"

extern float scale;
extern int width, height;

#define BECKETT_GAMENAME "Project Special K"
#define BECKETT_VERSIONJOKE "How Deep Does the Rabbit Hole Go"
#define BECKETT_SCREENWIDTH 1920
#define BECKETT_SCREENHEIGHT 1080

//Defines the standard interpolation mode for textures.
#define BECKETT_DEFAULTFILTER GL_LINEAR

//If defined, GamePrepSaveDirs must be provided.
#define BECKETT_EXTRASAVEDIRS

//If defined, adds Ambient and Speech volume controls alongside Music and Sound.
#define BECKETT_MOREVOLUME

//If defined, allows 3D positional audio. IF not, Audio::SetPosition is entirely removed.
#define BECKETT_3DAUDIO

//If defined, leaves out all 3D model stuff. Allows you to leave out UFBX, too.
//#define BECKETT_NO3DMODELS
//DO NOT ENABLE FOR PSK UNLESS YOU ARE A FUCKING MORON

//If defined, maps the left analog stick to the specified actions.
//Assumes the actions are in the order north-west-south-east.
#define BECKETT_ANALOGLEFT Binds::WalkN
//If defined, maps the right analog stick to the specified actions.
//Makes the same assumption as BECKETT_ANALOGLEFT.
//#define BECKETT_ANALOGRIGHT Binds::CameraUp

constexpr int MaxLights = 8;

struct Light
{
	glm::vec4 pos;
	glm::vec4 color;
};

struct CommonUniforms
{
	//Must match shaders/common.fs
	float TotalTime; //0
	float DeltaTime; //4
	float CurveAmount; //8
	float CurvePower; //12
	bool CurveEnabled; //16
	char _a[3];
	bool Toon; //20
	char _b[3];
	glm::uvec2 ScreenRes; //24
	glm::mat4 View; //32
	glm::mat4 Projection; //96
	glm::mat4 InvView; //160
	Light Lights[MaxLights]; //224
	glm::vec4 PlayerSkin; //480
	glm::vec4 PlayerEyes; //496
	glm::vec4 PlayerCheeks; //512
	glm::vec4 PlayerHair; //528
	glm::vec4 PlayerHairHi; //544
	float GrassColor; //560
	float TimeOfDay; //564
	int PostEffect; //568
};
extern CommonUniforms commonUniforms;

//BJTS functions that actually change the string content.
extern const std::map<std::string, BJTSFunc> bjtsPhase1;
//BJTS functions loaded from Lua scripts.
extern std::map<std::string, std::string> bjtsPhase1X;

class Audio;
extern std::map<std::string, std::map<std::string, std::shared_ptr<Audio>>> generalSounds;
