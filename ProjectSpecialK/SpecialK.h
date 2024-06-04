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

#define VTS

#ifdef VTS
#ifdef _WIN32
extern "C"
{
	//why bother including windows headers lol
	_declspec(dllimport) int __stdcall SetConsoleOutputCP(_In_ unsigned int wCodePageID);
	_declspec(dllimport) int __stdcall GetConsoleMode(_In_ void* hConsoleHandle, _Out_ unsigned long* lpMode);
	_declspec(dllimport) int __stdcall SetConsoleMode(_In_ void* hConsoleHandle, _In_ unsigned long dwMode);
	_declspec(dllimport) void* __stdcall GetStdHandle(_In_ unsigned long nStdHandle);

	_declspec(dllimport) int __stdcall AllocConsole(void);

	_declspec(dllimport) int __stdcall MessageBoxA(_In_opt_ void* hWnd, _In_opt_ const char* lpText, _In_opt_ const char* lpCaption, _In_ unsigned int uType);
	_declspec(dllimport) int __stdcall MessageBoxW(_In_opt_ void* hWnd, _In_opt_ const wchar_t* lpText, _In_opt_ const wchar_t* lpCaption, _In_ unsigned int uType);
	_declspec(dllimport) int __stdcall MultiByteToWideChar(_In_ unsigned int CodePage, _In_ unsigned long dwFlags, const char* lpMultiByteStr, _In_ int cbMultiByte, wchar_t* lpWideCharStr, _In_ int cchWideChar);
#define MessageBox MessageBoxW
}
#define STDOUT ((unsigned long)-11)
#endif
#define REDWARNING u8"\x1B[91m\u26D4\x1B[0m"
#define WARNING u8"\x1B[93m\u26A0\uFE0F\x1B[0m"
#define BOLD u8"\x1B[1m"
#define UNDERLINE u8"\x1B[4m"
#define NORMAL u8"\x1B[0m"
#define GREEN u8"\x1B[92m"
#define CYAN u8"\x1B[36m"
#define GRAY u8"\x1B[90m"
#else
#ifdef _WIN32
extern "C"
{
	_declspec(dllimport) int __stdcall SetConsoleOutputCP(_In_ unsigned int wCodePageID);
}
#endif
#define REDWARNING u8""
#define BOLD u8""
#define UNDERLINE u8""
#define NORMAL u8""
#define GREEN u8""
#define CYAN u8""
#endif

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

namespace UI
{
	extern std::map<std::string, glm::vec4> themeColors;
	extern std::vector<glm::vec4> textColors;

	extern JSONObject& json;
};
