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
	extern JSONObject& settings;
};

#define conprint(C, F, ...) console->Print(C, fmt::format(F, __VA_ARGS__))

//To make coordinate spaces more explicit, maybe?
#pragma warning(push)
#pragma warning(disable: 4455)
//Distance in pixels
inline constexpr int operator "" px(unsigned long long v) { return (int)v; }
//Distance in tiles
inline constexpr int operator "" t(unsigned long long v) { return (int)v; }
//Distance normalized to a 0.0-1.0 range
inline constexpr float operator "" pt(long double v) { return (float)v; }
#pragma warning(pop)


//Until I find out how the alternatives to std::vector that DO have find and contains work...
template<typename T>
auto vecfind(const std::vector<T>& h, const T& n)
{
	for (auto& x = h.begin(); x < h.end(); ++x)
		if ((*x).id == n.id)
			return x;
	return h.end();
};

template<typename T>
auto veccontains(const std::vector<T>& h, const T& n)
{
	for (const auto& x : h)
		if (x.id == n.id)
			return true;
	return false;
};

