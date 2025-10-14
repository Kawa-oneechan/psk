#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "engine/SpriteRenderer.h"
#include "engine/TextUtils.h"
#include "engine/Utilities.h"
#include "engine/Tickable.h"

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

//If defined, BJTS support is disabled entirely. Text strings are always effectively "raw".
//#define BECKETT_NOBJTS
//DO NOT ENABLE FOR PSK UNLESS YOU ARE A FUCKING MORON

//If defined, only US English text strings are acknowledged.
//#define BECKETT_ONLYMURCAN

//If defined, enables multi-sample anti-aliasing.
//#define BECKETT_MSAA

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
	float TotalTime;
	float DeltaTime;
	glm::uvec2 ScreenRes;
	glm::mat4 View;
	glm::mat4 Projection;
	glm::mat4 InvView;
	Light Lights[MaxLights];
	int PostEffect;
	float CurveAmount;
	float CurvePower;
	alignas(4) bool CurveEnabled;
	alignas(4) bool Toon;
	float GrassColor;
	float TimeOfDay;
	float HorizonPitch;
	glm::vec3 NightSkyColor;
	float Weather;
	glm::vec4 PlayerSkin;
	glm::vec4 PlayerEyes;
	glm::vec4 PlayerCheeks;
	glm::vec4 PlayerHair;
	glm::vec4 PlayerHairHi;
};
extern CommonUniforms commonUniforms;

//BJTS functions that actually change the string content.
extern const std::map<std::string, BJTSFunc> bjtsPhase1;
//BJTS functions loaded from Lua scripts.
extern std::map<std::string, std::string> bjtsPhase1X;

class Audio;
extern std::map<std::string, std::map<std::string, std::shared_ptr<Audio>>> generalSounds;
