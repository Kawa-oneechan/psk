#pragma once

#include <cstdio>
#include <ctime>
#include <cstdarg>
#include <vector>
#include <string>
#include <functional>
#include <stack>
#include <algorithm>
#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <JSON/JSON.h>
#include <format.h>
#include <sol.hpp>

#include "Tickable.h"

#include "VFS.h"
#include "Shader.h"
#include "Texture.h"
#include "SpriteRenderer.h"
#include "Camera.h"
#include "Audio.h"

#include "NameableThing.h"
#include "Item.h"
#include "Species.h"
#include "Traits.h"
#include "Villager.h"
#include "Player.h"
#include "Database.h"
#include "Text.h"
#include "Console.h"

#define VERSIONJOKE "Horkly Warding"

__declspec(noreturn)
extern void FatalError(const std::string& message);

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

enum class LoadSpawnChoice
{
	FrontDoor,
	MainRoom,
	LastBed,
	InPlace,
};

//Actual current screen width.
extern int width;
//Actual current screen height.
extern int height;
//Ratio of current screen height over 1080.
extern float scale;

extern Texture* whiteRect;

extern sol::state Sol;

namespace MeshBucket
{
	extern void Flush();
	extern void Draw(Model::Mesh& mesh, const glm::vec3& position, const glm::quat& rotation, const glm::mat4 bones[], size_t boneCt);
}

namespace rnd
{
	extern int getInt(int min, int max);
	extern int getInt(int max);
	extern int getInt();
	extern float getFloat(float min, float max);
	extern float getFloat(float max);
	extern float getFloat();
	extern bool flip();
}

namespace UI
{
	extern std::map<std::string, glm::vec4> themeColors;
	extern std::vector<glm::vec4> textColors;
	extern std::shared_ptr<Texture> controls;

	extern JSONObject json;
	extern JSONObject settings;
	extern std::string initFile;
};

#define conprint(C, F, ...) console->Print(C, fmt::format(F, __VA_ARGS__))
#ifdef DEBUG
#define debprint(C, F, ...) console->Print(C, fmt::format(F, __VA_ARGS__))
#else
#define debprint(C, F, ...)
#endif

#define BJTSParams const std::vector<std::string>& tags, int start, int len

//Shows a loading screen while running another thread.
extern void ThreadedLoader(std::function<void(float*)> loader);
