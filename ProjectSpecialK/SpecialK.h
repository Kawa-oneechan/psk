#pragma once

#include <cstdio>
#include <ctime>
#include <cstdarg>
#include <vector>
#include <string>
#include <functional>
#include <stack>
#include <algorithm>

#include "support/glm/glm.hpp"
#include "support/glm/gtc/matrix_transform.hpp"
#include "support/glm/gtc/type_ptr.hpp"
#include "support/JSON/JSON.h"
#include "support/format.h"
#include "support/sol.hpp"

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

using namespace std::literals;

#define VERSIONJOKE "Oh No, The Ground!"

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

template<typename T> static T clamp(T val, T minval, T maxval) { return std::max<T>(std::min<T>(val, maxval), minval); }

extern void GetAtlas(std::vector<glm::vec4> &ret, const std::string& jsonFile);

extern Shader* spriteShader;
extern Texture* whiteRect;

extern sol::state Sol;

using rune = unsigned int;
using hash = unsigned int;

//Given a JSON array with two numbers in it, returns a vec2 with those numbers.
extern glm::vec2 GetJSONVec2(JSONValue* val);
//Given a JSON array with three numbers in it, returns a vec3 with those numbers.
extern glm::vec3 GetJSONVec3(JSONValue* val);
//Given a JSON array with four numbers in it, returns a vec4 with those numbers.
extern glm::vec4 GetJSONVec4(JSONValue* val);
//Converts [R,G,B], [R,G,B,A], "#RRGGBB" or "#AARRGGBB" to glm::vec4.
//Returns an alpha of -1, which is impossible, on error.
extern glm::vec4 GetJSONColor(JSONValue* val);
//Given a JSON array with two numbers in it, returns a vec2 with those numbers.
//But given a string that parses as "mmm dd" or "mmm/dd", parses that as a day and returns those numbers.
//Either way, the first value is a day from 1-31 and the second a month from 1-12.
extern glm::vec2 GetJSONDate(JSONValue* val);

//Returns true if point is inside of polygon.
extern bool PointInPoly(const glm::vec2 point, const std::vector<glm::vec2>& polygon);
//Returns true if point is in rect.
extern bool PointInRect(const glm::vec2 point, const glm::vec4 rect);

std::string LoadCamera(JSONValue* json);
std::string LoadCamera(const std::string& path);

std::string LoadLights(JSONValue* json);
std::string LoadLights(const std::string& path);

//Decodes a UTF-8 byte sequence to a codepoint, returns it and the size of the sequence.
extern std::tuple<rune, size_t> GetChar(const std::string& what, size_t where);
//Encodes a codepoint into a UTF-8 byte sequence and appends it to the given string.
extern void AppendChar(std::string& where, rune what);

//Renders a set of tabular data to the console in a nice lined table.
extern void Table(std::vector<std::string> data, size_t stride);

//Returns a calendar date for things like "the fourth Friday in November".
tm* GetNthWeekdayOfMonth(int month, int dayOfWeek, int howManyth);

//Given a full path to a file ("data/foo/bar.txt"), returns the path part including the final separator ("data/foo/").
extern std::string GetDirFromFile(const std::string& path);

//Given a piece of code (shader?) that may contain "#include" statements and a search path, inserts the included files, 
extern void HandleIncludes(std::string& code, const std::string& path);

//Invokes Scale2x, 3x, or 4x on an image. Returned pixel data is the caller's responsibility to delete.
extern unsigned char* ScaleImage(unsigned char* original, int origWidth, int origHeight, int channels, int targetScale);

//Returns the name of a key for the given scancode, using glfwGetKeyName for printables and Text::Get for specials.
extern std::string GetKeyName(int scancode);

//Checks if a string contains only characters valid for an ID (alhpanumerics, colons, underscores).
extern bool IsID(const std::string& id);
//Checks if a string contains a colon, which would mark it as a valid ID.
extern bool IDIsQualified(const std::string& id);
//Prepends the given namespace to an ID.
extern std::string Qualify(const std::string& id, const std::string& ns);
//Removes the frontmost namespace from an ID.
extern std::string UnQualify(const std::string& id);

namespace NookCode
{
	extern std::string Encode(std::array<unsigned char, 8>& d);
	extern std::string Encode(hash itemHash, int variant, int pattern);
	extern std::array<unsigned char, 8> Decode(const std::string& code);
	extern void Decode(const std::string& code, hash& itemHash, int& variant, int& pattern);
}

namespace MeshBucket
{
	extern void Flush();
	extern void Draw(unsigned int vao, TextureArray* textures[], int layer, Shader* shader, const glm::vec3& position, const glm::quat& rotation, const glm::mat4 bones[], size_t indices, size_t boneCt);
}

//Returns the CRC32 hash for the given text.
extern hash GetCRC(const std::string& text);
//Returns the CRC32 hash for the given data.
extern hash GetCRC(unsigned char *buffer, int len);

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

#define MSBTParams const std::vector<std::string>& tags, int start, int len

//Shows a loading screen while running another thread.
extern void ThreadedLoader(std::function<void(float*)> loader);

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


//Splits a string into a vector of strings by the specified delimiter.
std::vector<std::string> Split(std::string& data, char delimiter);

//Changes a string's characters to lowercase, in place.
extern void StringToLower(std::string& data);

//Changes a string's characters to uppercase, in place.
extern void StringToUpper(std::string& data);

//Removes spaces from a string, in place.
extern void StripSpaces(std::string& data);

//Finds and replaces all instances of a thing in a string, in place.
extern void ReplaceAll(std::string& data, const std::string& find, const std::string& replace);

//Removes MSBT tags from a string.
extern std::string StripMSBT(const std::string& data);

template<typename T>
auto StringToEnum(const std::string& s, std::initializer_list<const std::string> opts)
{
	auto it = std::find(opts.begin(), opts.end(), s);
	if (it == opts.end())
		throw std::range_error(fmt::format("StringToEnum: can't find \"{}\" in list \"{}\".", s, join(opts.begin(), opts.end())));
	return (T)std::distance(opts.begin(), it);
}

template<typename InputIt>
std::string join(InputIt begin, InputIt end, const std::string& separator = ", ", const std::string& concluder = "")
{
	std::ostringstream ss;

	if (begin != end)
		ss << *begin++;

	while (begin != end)
	{
		ss << separator;
		ss << *begin++;
	}

	ss << concluder;
	return ss.str();
}
