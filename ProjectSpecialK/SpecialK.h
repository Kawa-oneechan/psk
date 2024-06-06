#pragma once

#include <cstdio>
#include <ctime>
#include <cstdarg>
#include <vector>
#include <string>
#include <functional>
#include <stack>
#include <algorithm>

#include "support/glad/glad.h"
#include <GLFW/glfw3.h>

#include "support/glm/glm.hpp"
#include "support/glm/gtc/matrix_transform.hpp"
#include "support/glm/gtc/type_ptr.hpp"
#include "support/stb_image.h"
#include "support/JSON/JSON.h"
#include "support/format.h"
#include "support/sol.hpp"
#include "support/tweeny-3.2.0.h"

#include "VFS.h"
#include "Shader.h"
#include "Texture.h"
#include "SpriteRenderer.h"
#include "Camera.h"
#include "Audio.h"

#include "Item.h"
#include "Tickable.h"
#include "Species.h"
#include "Traits.h"
#include "Villager.h"
#include "Player.h"
#include "Database.h"
#include "Text.h"
#include "Console.h"

using namespace std::literals;

__declspec(noreturn)
extern void FatalError(const std::string& message);

typedef enum
{
	FrontDoor,
	MainRoom,
	LastBed,
	InPlace,
} LoadSpawnChoice;

extern float width, height;
extern float scale;

template<typename T> static T clamp(T val, T minval, T maxval) { return std::max<T>(std::min<T>(val, maxval), minval); }

typedef std::vector<glm::vec4> TextureAtlas;

extern void GetAtlas(TextureAtlas &ret, const std::string& jsonFile);

extern SpriteRenderer* sprender;
extern Shader* spriteShader;
extern Texture* whiteRect;

extern sol::state Sol;

extern glm::vec2 GetJSONVec2(JSONValue* val);
extern glm::vec4 GetJSONVec4(JSONValue* val);

extern bool PointInPoly(const glm::vec2 point, const std::vector<glm::vec2>& polygon);
extern bool PointInRect(const glm::vec2 point, const glm::vec4 rect);

namespace UI
{
	extern std::map<std::string, glm::vec4> themeColors;
	extern std::vector<glm::vec4> textColors;

	extern JSONObject& json;
};

#define conprint(C, F, ...) console->Print(C, fmt::format(F, __VA_ARGS__));
