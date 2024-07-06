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

enum class LoadSpawnChoice
{
	FrontDoor,
	MainRoom,
	LastBed,
	InPlace,
};

extern float width, height;
extern float scale;

template<typename T> static T clamp(T val, T minval, T maxval) { return std::max<T>(std::min<T>(val, maxval), minval); }

extern void GetAtlas(std::vector<glm::vec4> &ret, const std::string& jsonFile);

extern SpriteRenderer* sprender;
extern Shader* spriteShader;
extern Texture* whiteRect;

extern sol::state Sol;

extern glm::vec2 GetJSONVec2(JSONValue* val);
extern glm::vec4 GetJSONVec4(JSONValue* val);
//Converts [R,G,B], [R,G,B,A], "#RRGGBB" or "#AARRGGBB" to glm::vec4.
//Returns an alpha of -1, which is impossible, on error.
extern glm::vec4 GetJSONColor(JSONValue* val);

extern bool PointInPoly(const glm::vec2 point, const std::vector<glm::vec2>& polygon);
extern bool PointInRect(const glm::vec2 point, const glm::vec4 rect);

//Decodes a UTF-8 byte sequence to a codepoint, returns it and the size of the sequence.
extern std::tuple<unsigned int, size_t> GetChar(const std::string& what, size_t where);
//Encodes a codepoint into a UTF-8 byte sequence and appends it to the given string.
extern void AppendChar(std::string& where, unsigned int what);

extern void Table(std::vector<std::string> data, size_t stride);

extern unsigned int GetCRC(const std::string& text);
extern unsigned int GetCRC(unsigned char *buffer, int len);

namespace UI
{
	extern std::map<std::string, glm::vec4> themeColors;
	extern std::vector<glm::vec4> textColors;
	extern std::shared_ptr<Texture> controls;

	extern JSONObject json;
	extern JSONObject settings;
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

template<typename T>
auto StringToEnum(const std::string& s, std::initializer_list<const std::string> opts)
{
	auto it = std::find(opts.begin(), opts.end(), s);
	if (it == opts.end())
		throw std::range_error(fmt::format("StringToEnum: can't find \"{}\" in list \"{}\".", s, join(opts.begin(), opts.end())));
	return (T)std::distance(opts.begin(), it);
}
