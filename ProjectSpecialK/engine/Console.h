#pragma once
#include <functional>
#include <fstream>
#include <format.h>
#include <glm/glm.hpp>
#include "Tickable.h"
#include "JsonUtils.h"

class TextField;

struct CVar
{
	std::string name;
	enum class Type
	{
		Bool, Int, Float, String, Vec2, Vec3, Vec4, Color
	} type;
	union
	{
		void* asVoid;
		bool* asBool;
		int* asInt;
		float* asFloat;
		std::string* asString;
		glm::vec2* asVec2;
		glm::vec3* asVec3;
		glm::vec4* asVec4;
	};
	bool cheat;
	int min, max;
	std::string description;

	bool Set(const std::string& value);
	std::string ToString();
};

struct CCmd
{
	std::string name;
	std::function<void(jsonArray& args)> act;
	std::string description;
};

class Console : public Tickable
{
private:
	std::vector<std::pair<int, std::string>> buffer;
	std::vector<std::string> history;
	int historyCursor;
	int scrollCursor;
	TextField* inputLine;
	float timer;
	int appearState;

	std::ofstream hardcopy;

public:
	std::vector<CVar> cvars;
	std::vector<CCmd> ccmds;
	bool visible;

	Console();
	bool Execute(const std::string& str);
	void Print(int color, const std::string& str);
	void Print(const std::string& str);
	void Flush();
	bool Character(unsigned int codepoint);
	bool Scancode(unsigned int scancode);
	void Open();
	void Close();
	bool Tick(float dt);
	void Draw(float dt);
	void RegisterCVar(const std::string& name, CVar::Type type, void* target, bool cheat = false, int min = -1, int max = -1);
	void RegisterCCmd(const std::string& name, std::function<void(jsonArray& args)> act);

	static bool CheckSplat(const std::string& pattern, const std::string& text);
};

extern Console* console;

#define conprint(C, F, ...) console->Print(C, fmt::format(F, __VA_ARGS__))
#ifdef DEBUG
#define debprint(C, F, ...) console->Print(C, fmt::format(F, __VA_ARGS__))
#else
#define debprint(C, F, ...)
#endif
