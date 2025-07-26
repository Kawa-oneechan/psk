#pragma once
#include <functional>
#include <fstream>
#include <JSON/JSONValue.h>
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

	bool Set(const std::string& value)
	{
		auto json = JSON::Parse(value.c_str());
		switch (type)
		{
		case Type::Bool:
			if (json->IsNumber())
				*asBool = json->AsInteger() != 0;
			else if (json->IsBool())
				*asBool = json->AsBool();
			return true;
		case Type::Int:
			if (json->IsNumber())
			{
				auto i = json->AsInteger();
				if (!(min == -1 && max == -1))
					i = glm::clamp(i, min, max);
				*asInt = i;
			}
			return true;
		case Type::Float:
			if (json->IsNumber())
			{
				auto i = json->AsNumber();
				if (!(min == -1 && max == -1))
					i = glm::clamp(i, (float)min, (float)max);
				*asFloat = i;
			}
			return true;
		case Type::String:
			if (json->IsNumber())
				*asString = fmt::format("{}", json->AsNumber());
			else if (json->IsString())
				*asString = json->AsString();
			return true;
		case Type::Vec2:
			if (json->IsArray())
				*asVec2 = GetJSONVec2(json);
			return true;
		case Type::Vec3:
			if (json->IsArray())
				*asVec3 = GetJSONVec3(json);
			return true;
		case Type::Vec4:
			if (json->IsArray())
				*asVec4 = GetJSONVec4(json);
			return true;
		case Type::Color:
			*asVec4 = GetJSONColor(json);
			return true;
		}
		return false;
	}
};

struct CCmd
{
	std::string name;
	std::function<void()> act;
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
	void RegisterCCmd(const std::string& name, std::function<void()> act);
};

extern Console* console;

#define conprint(C, F, ...) console->Print(C, fmt::format(F, __VA_ARGS__))
#ifdef DEBUG
#define debprint(C, F, ...) console->Print(C, fmt::format(F, __VA_ARGS__))
#else
#define debprint(C, F, ...)
#endif
