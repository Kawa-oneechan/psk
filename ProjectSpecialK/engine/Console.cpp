#include <glad/glad.h>
#include <sol.hpp>

#include "Console.h"
#include "TextField.h"
#include "TextUtils.h"
#include "../Game.h"

//For version information
#include <miniz.h>
#include <fmodex/fmod.hpp>
#ifdef DEBUG
#include <ImGUI/imgui.h>
#endif
#include <ufbx.h>
extern "C" { const char* glfwGetVersionString(void); }

extern void ConsoleRegister(Console* console);

extern sol::state Sol;
extern Texture* whiteRect;

extern bool cheatsEnabled;
extern float timeScale;

static void CCmdVersion(jsonArray& args);
static void CCmdCVarList(jsonArray& args);
static void CCmdCmdList(jsonArray& args);
static void CCmdCRC32(jsonArray& args);

void Console::predict()
{
	prediction.clear();
	if (!inputLine->value.empty())
	{
		auto& input = inputLine->value;
		for (const auto& cc : ccmds)
		{
			if (cc.name.length() >= input.length() && cc.name.substr(0, input.length()) == input)
			{
				prediction = cc.name;
				return;
			}
		}
		for (const auto& cv : cvars)
		{
			if (cv.name.length() >= input.length() && cv.name.substr(0, input.length()) == input)
			{
				prediction = cv.name;
				return;
			}
		}
	}
}

Console::Console()
{
	visible = false;

	hardcopy = std::ofstream("console.log", std::ios::trunc);

	Print(3, BECKETT_GAMENAME);
	Print(3, "-------------------------");

	inputLine = new TextField();
	//inputLine->rect = glm::vec4(16, (height / 3) - 24, width - 8, 20);
	inputLine->font = 0;
	inputLine->Clear();

	history.clear();
	historyCursor = 0;
	scrollCursor = 0;

	timer = 0.0f;
	appearState = 0;

	RegisterCCmd("clear", [&](jsonArray&) { buffer.clear(); scrollCursor = 0; });
	RegisterCCmd("version", CCmdVersion);
	RegisterCCmd("cvarlist", CCmdCVarList);
	RegisterCCmd("cmdlist", CCmdCmdList);
	RegisterCCmd("crc32", CCmdCRC32);
	RegisterCVar("sv_cheats", CVar::Type::Bool, &cheatsEnabled);
	RegisterCVar("timescale", CVar::Type::Float, &timeScale, true);
	ConsoleRegister(this);
}

void Console::Print(int color, const std::string& str)
{
	if (str.find('\n') != std::string::npos)
	{
		auto lines = Split((std::string&)str, '\n');
		for (auto& line : lines)
			Print(color, line);
		return;
	}

	buffer.emplace_back(std::make_pair(glm::clamp(color, 0, 8), str));
	if (hardcopy.good())
	{
		hardcopy.write(str.c_str(), str.length());
		hardcopy.write("\n", 1);
	}
}

void Console::Print(const std::string& str)
{
	Print(0, str);
}

void Console::Flush()
{
	if (hardcopy.good())
		hardcopy.flush();
}

bool Console::Execute(const std::string& str)
{
	auto first = std::string(str);
	auto second = std::string("");
	auto haveArgs = false;
	{
		auto space = first.find(' ');
		if (space != std::string::npos)
		{
			first = first.substr(0, space);
			second = str.substr(space + 1);
			haveArgs = true;
		}
	}

	for (auto& cv : cvars)
	{
		if (cv.name == first)
		{
			if (!haveArgs)
			{
				Print(0, fmt::format("{} is {}", cv.name, cv.ToString()));
				return true;
			}
			else
			{
				if (cv.cheat && !cheatsEnabled)
				{
					Print(1, fmt::format("Changing {} is considered a cheat.", cv.name));
					return false;
				}
				if (cv.Set(second))
				{
					Print(0, fmt::format("{} set to {}", cv.name, cv.ToString()));
					return true;
				}
				else
				{
					Print(2, fmt::format("Could not set cvar {} to {}", cv.name, second));
					return false;
				}
			}
		}
	}
	for (auto& cc : ccmds)
	{
		if (cc.name == first)
		{
			try
			{
				cc.act(json5pp::parse5(fmt::format("[ {} ]", second)).as_array());
				return true;
			}
			catch (json5pp::syntax_error& x)
			{
				/*
				std::string what = x.what();
				if (what.find("illegal character") != -1 && what.find("in array") != -1)
				{
					//Print(5, "Did you mean to put that in quotes?");
					Execute(fmt::format("{} \"{}\"", first, second));
				}
				else
					Print(1, what);
				*/
				Print(1, x.what());
				return false;
			}
		}
	}

	try
	{
		Sol.script(str);
	}
	catch (sol::error& e)
	{
		std::string what = e.what();
		if (what.find("attempt to yield from outside a coroutine") != -1)
			return true; //Accept this silently.
		else
			conprint(1, "Error: {}", what);
	}
	return false;
}

bool Console::Character(unsigned int codepoint)
{
	auto ret = inputLine->Character(codepoint);
	predict();
	return ret;
}

bool Console::Scancode(unsigned int scancode)
{
	if (scancode == 28) //enter
	{
		if (history.size() == 0 || history.back() != inputLine->value)
			history.emplace_back(inputLine->value);
		historyCursor = 0;
		Print(8, fmt::format("]{}", inputLine->value));
		Execute(inputLine->value);
		inputLine->Clear();
	}
	else if (scancode == 328) //up
	{
		if (historyCursor < history.size())
		{
			historyCursor++;
			inputLine->Set(history[history.size() - historyCursor]);
		}
	}
	else if (scancode == 336) //down
	{
		if (historyCursor > 1)
		{
			historyCursor--;
			inputLine->Set(history[history.size() - historyCursor]);
		}
		else
		{
			historyCursor = 0;
			inputLine->Clear();
		}
	}
	else if (scancode == 329) //pageup
	{
		if (scrollCursor < buffer.size())
		{
			scrollCursor += 5;
			if (scrollCursor >= buffer.size())
				scrollCursor = (int)buffer.size() - 1;
		}
	}
	else if (scancode == 337) //pagedown
	{
		if (scrollCursor > 0)
		{
			scrollCursor -= 5;
			if (scrollCursor < 0)
				scrollCursor = 0;
		}
	}

	auto ret = inputLine->Scancode(scancode);
	predict();

	if (scancode == 15) //tab
	{
		inputLine->Set(prediction);
	}

	return ret;
}

void Console::Open()
{
	appearState = 1;
	visible = true;
}

void Console::Close()
{
	appearState = 2;
	visible = true;
}

bool Console::Tick(float dt)
{
	if (!visible)
		return false;

	if (appearState == 1 && timer < 1.0f)
	{
		timer += dt * 3.0f;
		if (timer >= 1.0f)
			appearState = 0;
	}
	else if (appearState == 2 && timer > 0.0f)
	{
		timer -= dt * 3.0f;
		if (timer <= 0.0f)
		{
			appearState = 0;
			visible = false;
		}
	}
	timer = glm::clamp(timer, 0.0f, 1.0f);

	return inputLine->Tick(dt);
}

void Console::Draw(float dt)
{
	static glm::vec4 colors[] =
	{
		glm::vec4(0.5, 0.5, 0.5, 1),
		glm::vec4(1, 0, 0, 1),
		glm::vec4(0, 1, 0, 1),
		glm::vec4(1, 1, 0, 1),
		glm::vec4(0, 0, 1, 1),
		glm::vec4(1, 0, 1, 1),
		glm::vec4(0, 1, 1, 1),
		glm::vec4(0.75, 0.75, 0.75, 1),
		glm::vec4(1, 1, 1, 1),
	};

	if (!visible)
		return;

	auto h = (float)height / 3;
	glm::vec2 offset{ 0 };
	if (appearState != 0)
		offset.y += glm::mix(-h, 0.0f, timer);

	Sprite::DrawSprite(*whiteRect, offset, glm::vec2(width, h), glm::vec4(0), 0, glm::vec4(0, 0, 0, 0.8));

	auto pos = offset + glm::vec2(4, h - 42);
	for (unsigned int i = (unsigned int)buffer.size() - scrollCursor, lines = 21; i-- > 0 && lines-- > 0;)
	{
		Sprite::DrawText(0, buffer[i].second, pos, colors[buffer[i].first], 100.0f, 0, true);
		pos.y -= 15;
	}

	inputLine->rect = glm::vec4(16, offset.y + (h - 24), width - 8, offset.y + (h - 24) + 20);
	Sprite::DrawText(0, "]", glm::vec2(4, inputLine->rect.y));
	if (!prediction.empty())
		Sprite::DrawText(0, prediction, glm::vec2(inputLine->rect.x, inputLine->rect.y), glm::vec4(0.5f));
	inputLine->Draw(dt);
}

void Console::RegisterCVar(const std::string& name, CVar::Type type, void* target, bool cheat, int min, int max)
{
	for (auto& cv : cvars)
	{
		if (cv.name == name)
		{
			cv.type = type;
			cv.asVoid = target;
			cv.cheat = cheat;
			return;
		}
	}
	CVar cv;
	cv.name = name;
	cv.type = type;
	cv.asVoid = target;
	cv.cheat = cheat;
	cv.min = min;
	cv.max = max;
	cvars.push_back(cv);
}

void Console::RegisterCCmd(const std::string& name, std::function<void(jsonArray& args)> act)
{
	for (auto& cc : ccmds)
	{
		if (cc.name == name)
		{
			cc.act = act;
			return;
		}
	}
	CCmd cc;
	cc.name = name;
	cc.act = act;
	ccmds.push_back(cc);
}

bool Console::CheckSplat(const std::string& pattern, const std::string& text)
{
	if (pattern.empty() || text.empty())
		return true;
	std::function<bool(const char*, const char*)> splat;
	splat = [&](const char* p, const char* t) -> bool
	{
		while (*p)
		{
			if (*p == '*')
			{
				char s = *++p;
				while (*t && *t != s) t++;
				if (*t && *t == s)
				{
					if (splat(p, t++)) return true;
					p--;
				}
			}
			else if (*p == '?' || *p == *t)
			{
				p++; t++;
			}
			else
				return false;
		}
		return (*p | *t) == 0;
	};
	return splat(pattern.c_str(), text.c_str());
}

bool CVar::Set(const std::string& value)
{
	{
		if (type == Type::String && value[0] != '\"')
			return Set(fmt::format("\"{}\"", value));
		if ((type == Type::Vec2 || type == Type::Vec3 || type == Type::Vec4) && value[0] != '[')
			return Set(fmt::format("[{}]", value));
		if (type == Type::Color)
		{
			if (value[0] == '#')
				return Set(fmt::format("\"{}\"", value));
			if (value[0] != '[')
				return Set(fmt::format("[{}]", value));
		}

		auto json = json5pp::parse5(value);
		switch (type)
		{
		case Type::Bool:
			if (json.is_number())
				*asBool = json.as_integer() != 0;
			else if (json.is_boolean())
				*asBool = json.as_boolean();
			return true;
		case Type::Int:
			if (json.is_integer())
			{
				auto i = json.as_integer();
				if (!(min == -1 && max == -1))
					i = glm::clamp(i, min, max);
				*asInt = i;
			}
			return true;
		case Type::Float:
			if (json.is_number())
			{
				auto i = (float)json.as_number();
				if (!(min == -1 && max == -1))
					i = glm::clamp(i, (float)min, (float)max);
				*asFloat = i;
			}
			return true;
		case Type::String:
			if (json.is_number())
				*asString = fmt::format("{}", json.as_number());
			else if (json.is_string())
				*asString = json.as_string();
			return true;
		case Type::Vec2:
			if (json.is_array())
				*asVec2 = GetJSONVec2(json);
			return true;
		case Type::Vec3:
			if (json.is_array())
				*asVec3 = GetJSONVec3(json);
			return true;
		case Type::Vec4:
			if (json.is_array())
				*asVec4 = GetJSONVec4(json);
			return true;
		case Type::Color:
			*asVec4 = GetJSONColor(json);
			return true;
		}
		return false;
	}
}

std::string CVar::ToString()
{
	switch (type)
	{
	case CVar::Type::Bool: return fmt::format("{}", *asBool);
	case CVar::Type::Int: return fmt::format("{}", *asInt);
	case CVar::Type::Float: return fmt::format("{}", *asFloat);
	case CVar::Type::String: return fmt::format("\"{}\"", *asString);
	case CVar::Type::Vec2: return fmt::format("[{}, {}]", asVec2->x, asVec2->y);
	case CVar::Type::Vec3: return fmt::format("[{}, {}, {}]", asVec3->x, asVec3->y, asVec3->z);
	case CVar::Type::Color:
	case CVar::Type::Vec4: return fmt::format("[{}, {}, {}, {}]", asVec4->x, asVec4->y, asVec4->z, asVec4->w);
	}
	return "something";
}

static void CCmdVersion(jsonArray& args)
{
	args;
	conprint(8, BECKETT_GAMENAME " - " BECKETT_VERSIONJOKE);
#ifdef DEBUG
	conprint(7, "Debug version: " __DATE__);
#endif
#ifdef _MSC_VER
	conprint(7, "Microsoft Visual C++ version {}.{}.", (_MSC_VER / 100), (_MSC_VER % 100));
#endif
	conprint(7, "OpenGL: {}", glGetString(GL_VERSION));
	conprint(7, "Vendor: {}", glGetString(GL_VENDOR));
	conprint(7, "GLSL {}", glGetString(GL_SHADING_LANGUAGE_VERSION));
	conprint(7, "Renderer: {}", glGetString(GL_RENDERER));
	conprint(7, "GLFW: {}", glfwGetVersionString());
	conprint(7, "UFBX: {}.{}.{}", ufbx_version_major(UFBX_VERSION), ufbx_version_minor(UFBX_VERSION), ufbx_version_patch(UFBX_VERSION));
	conprint(7, "MiniZip: " MZ_VERSION);
	conprint(7, "FMOD: {}.{}.{}", (FMOD_VERSION >> 16) & 0xFFFF, (FMOD_VERSION >> 8) & 0xFF, FMOD_VERSION & 0xFF);
	conprint(7, "STB_Image: 2.30");
	conprint(7, "STB_Image_Write: 1.16");
	conprint(7, "STB_TrueType: 1.26");
#ifdef DEBUG
	conprint(7, "ImGUI version: " IMGUI_VERSION);
#endif
}

static void CCmdCVarList(jsonArray& args)
{
	if (args.size() != 0 && !args[0].is_string())
	{
		conprint(0, "Pattern must be a string.");
		return;
	}
	size_t results = 0;

	for (const auto& cv : console->cvars)
	{
		if (args.size() == 0 || Console::CheckSplat(args[0].as_string(), cv.name))
		{
			switch (cv.type)
			{
			case CVar::Type::Bool: conprint(0, "{} : {}", cv.name, *cv.asBool); break;
			case CVar::Type::Int: conprint(0, "{} : {}", cv.name, *cv.asInt); break;
			case CVar::Type::Float: conprint(0, "{} : {}", cv.name, *cv.asFloat); break;
			case CVar::Type::String: conprint(0, "{} : \"{}\"", cv.name, *cv.asString); break;
			case CVar::Type::Vec2: conprint(0, "{} : [{}, {}]", cv.name, cv.asVec2->x, cv.asVec2->y); break;
			case CVar::Type::Vec3: conprint(0, "{} : [{}, {}, {}]", cv.name, cv.asVec3->x, cv.asVec3->y, cv.asVec3->z); break;
			case CVar::Type::Color:
			case CVar::Type::Vec4: conprint(0, "{} : [{}, {}, {}, {}]", cv.name, cv.asVec4->x, cv.asVec4->y, cv.asVec4->z, cv.asVec4->w); break;
			}
			results++;
		}
	}
	conprint(0, "{} cvars", results);
}

static void CCmdCmdList(jsonArray& args)
{
	if (args.size() != 0 && !args[0].is_string())
	{
		conprint(0, "Pattern must be a string.");
		return;
	}
	size_t results = 0;
	for (const auto& cc : console->ccmds)
	{
		if (args.size() == 0 || Console::CheckSplat(args[0].as_string(), cc.name))
		{
			conprint(0, "{}", cc.name);
			results++;
		}
	}
	conprint(0, "{} ccmds", results);
}

static void CCmdCRC32(jsonArray& args)
{
	if (args.size() == 0 || !args[0].is_string())
	{
		conprint(0, "Input value must be a string.");
		return;
	}
	conprint(0, "{:X}", GetCRC(args[0].as_string()));
}
