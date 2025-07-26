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

#include "engine/Tickable.h"
#include "engine/VFS.h"
#include "engine/Shader.h"
#include "engine/Texture.h"
#include "engine/SpriteRenderer.h"
#include "engine/Console.h"
#include "Camera.h"
#include "Audio.h"

#include "NameableThing.h"
#include "Item.h"
#include "Species.h"
#include "Traits.h"
#include "Villager.h"
#include "Player.h"
#include "Database.h"

__declspec(noreturn)
extern void FatalError(const std::string& message);

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

//Shows a loading screen while running another thread.
extern void ThreadedLoader(std::function<void(float*)> loader);
