#pragma once

#include "SpecialK.h"

#include <fstream>

class TextField;

extern glm::vec2 GetJSONVec2(JSONValue* val);
extern glm::vec3 GetJSONVec3(JSONValue* val);
extern glm::vec4 GetJSONVec4(JSONValue* val);
extern glm::vec4 GetJSONColor(JSONValue* val);

struct CVar
{
	std::string name;
	enum class CVarType
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

	bool Set(const std::string& value)
	{
		auto json = JSON::Parse(value.c_str());
		switch (type)
		{
		case CVarType::Bool:
			if (json->IsNumber())
				*asBool = (int)(json->AsNumber()) != 0;
			else if (json->IsBool())
				*asBool = json->AsBool();
			return true;
		case CVarType::Int:
			if (json->IsNumber())
				*asInt = (int)(json->AsNumber());
			return true;
		case CVarType::Float:
			if (json->IsNumber())
				*asFloat = (float)json->AsNumber();
			return true;
		case CVarType::String:
			if (json->IsNumber())
				*asString = fmt::format("{}", json->AsNumber());
			else if (json->IsString())
				*asString = json->AsString();
			return true;
		case CVarType::Vec2:
			if (json->IsArray())
				*asVec2 = GetJSONVec2(json);
			return true;
		case CVarType::Vec3:
			if (json->IsArray())
				*asVec3 = GetJSONVec3(json);
			return true;
		case CVarType::Vec4:
			if (json->IsArray())
				*asVec4 = GetJSONVec4(json);
			return true;
		case CVarType::Color:
			*asVec4 = GetJSONColor(json);
			return true;
		}
		return false;
	}
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

	std::vector<CVar> cvars;

public:
	bool visible;

	Console();
	bool Execute(const std::string& str);
	void Print(int color, const std::string& str);
	void Print(const std::string& str);
	bool Character(unsigned int codepoint);
	bool Scancode(unsigned int scancode);
	void Open();
	void Close();
	void Tick(float dt);
	void Draw(float dt);
	void RegisterCVar(const std::string& name, CVar::CVarType type, void* target);
};
extern Console* console;
