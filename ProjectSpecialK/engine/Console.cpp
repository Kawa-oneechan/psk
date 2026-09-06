#include <glad/glad.h>

#include "Console.h"
#include "TextField.h"
#include "TextUtils.h"
#include "Game.h"
#include "InputsMap.h"
#include "CrcUtils.h"
#include "Font.h"
#include "../Game.h"

//For version information
#include <miniz.h>
#include <soloud/soloud.h>
#ifdef DEBUG
#include <ImGUI/imgui.h>
#endif
#ifndef BECKETT_NO3DMODELS
#include <ufbx.h>
#endif
extern "C" { const char* glfwGetVersionString(void); }

extern Texture* whiteRect;

extern bool cheatsEnabled;
extern float timeScale;
extern float fieldOfView, nearPlane, farPlane;

extern void RecalcProjections();

static void CCmdPrint(const jsonArray& args);
static void CCmdWait(const jsonArray& args);
static void CCmdAlias(const jsonArray& args);
static void CCmdHelp(const jsonArray& args);
static void CCmdVersion(const jsonArray& args);
static void CCmdCVarList(const jsonArray& args);
static void CCmdCmdList(const jsonArray& args);
static void CCmdCRC32(const jsonArray& args);

void Console::predict()
{
	prediction.clear();
	if (!inputLine->value.empty())
	{
		const auto& input = inputLine->value;
		auto pred = [input](auto& e)
		{
			return e.name.length() >= input.length() && e.name.substr(0, input.length()) == input;
		};
		auto it1 = std::find_if(ccmds.cbegin(), ccmds.cend(), pred);
		if (it1 != ccmds.cend())
		{
			prediction = it1->name;
			return;
		}
		auto it2 = std::find_if(cvars.cbegin(), cvars.cend(), pred);
		if (it2 != cvars.cend())
		{
			prediction = it2->name;
			return;
		}
	}
}

static void recalcProj(CVar*)
{
	RecalcProjections();
}

static std::string quake2json(const std::string& input)
{
	size_t cursor = 0;
	bool quote = false;
	std::string token{};
	std::string out{};
	std::string in{ input + " " };
	
	//not using "for (auto ch : testInput)" not because we'll need Beckett's UTF8 parser
	//but because we need the ability to look ahead for \".
	while (cursor < in.length())
	{
		auto ch = in[cursor];
		if (ch == '\\' && in[cursor + 1] == '"')
		{
			token += "\\\"";
			cursor += 2;
			continue;
		}
		if (ch == '"')
		{
			quote = !quote;
			cursor++;
			continue;
		}
		if (ch == '[')
		{
			out += "[ ";
			cursor++;
			continue;
		}
		if ((ch == ' ' || ch == ',' || ch == ']') && !quote)
		{
			if (ch == ']' && token.empty())
			{
				if (out.length() > 2)
				{
					//strip off final ", "
					out.erase(out.end() - 1);
					out.erase(out.end() - 1);
				}
				out += " ], ";
				cursor++;
				continue;
			}
			if (!token.empty())
			{
				//decide on type
				bool needsQuotes = true;
				if (token == "true" || token == "false")
					needsQuotes = false; //keywords
				else if (token.length() > 2 && token[0] == '0' && token[1] == 'x')
					needsQuotes = std::any_of(token.cbegin() + 2, token.cend(), [](auto c) { return !isxdigit(c); });
				else
					needsQuotes = std::any_of(token.cbegin(), token.cend(), [](auto c) { return !(isdigit(c) || c == '.'); });

				if (needsQuotes)
					token = "\"" + token + "\"";
				if (ch == ']')
					token += ']';
				out += token + ", ";
				token.clear();
			}
			cursor++;
			continue;
		}
		token += ch;
		cursor++;
	}

	//strip off final ", "
	out.erase(out.end() - 1);
	out.erase(out.end() - 1);
	return "[ " + out + " ]";
}

static bool checkSplat(const std::string& pattern, const std::string& text)
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

Console::Console() : hardcopy(std::ofstream("console.log", std::ios::trunc))
{
	visible = false;

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

	RegisterCCmd("print", CCmdPrint, "Prints a text string, optionally preceded by a color number.");
	RegisterCCmd("wait", CCmdWait, "Delays execution of console input.");
	RegisterCCmd("clear", [&](const jsonArray&) { buffer.clear(); scrollCursor = 0; }, "Clears the console buffer.");
	RegisterCCmd("alias", CCmdAlias, "Binds a console input to a shorter name.");
	RegisterCCmd("help", CCmdHelp, "You just used it.");
	RegisterCCmd("version", CCmdVersion, "Displays version and build information.");
	RegisterCCmd("cvarlist", CCmdCVarList, "Lists all console variables.");
	RegisterCCmd("cmdlist", CCmdCmdList, "Lists all console commands.");
	RegisterCCmd("crc32", CCmdCRC32, true, "Calculates a hash.");
	RegisterCVar("sv_cheats", CVar::Type::Bool, &cheatsEnabled, "Enables cheats.");
	RegisterCVar("in_deadzone", CVar::Type::Float, &Inputs.Deadzone, "The deadzone for analog sticks.");
	RegisterCVar("in_runthreshold", CVar::Type::Float, &Inputs.RunThreshold, "How far the analog stick has to go to count as running.");
	RegisterCVar("timescale", CVar::Type::Float, &timeScale, CVar::Flags::Cheat, "How fast time should go.");
	RegisterCVar("r_fov", CVar::Type::Float, &fieldOfView, CVar::Flags::Normal, 10, 160, recalcProj, "Field of view in degrees.");
	RegisterCVar("r_nearz", CVar::Type::Float, &nearPlane, CVar::Flags::Normal, -1, 10, recalcProj, "Near clipping plane.");
	RegisterCVar("r_farz", CVar::Type::Float, &farPlane, CVar::Flags::Normal, 1, 1000, recalcProj, "Far clipping plane.");
}

void Console::Print(int color, const std::string& str)
{
	if (str.find('\n') != std::string::npos)
	{
		auto lines = Split(std::decay_t<std::string>(str), '\n');
		for (const auto& line : lines)
			Print(color, line);
		return;
	}

	if (color >= 0)
		buffer.emplace_back(std::make_pair(glm::clamp(color, 0, 8), str));

	if (hardcopy.good())
	{
		hardcopy.write(str.c_str(), str.length());
		hardcopy.write("\n", 1);
	}
}

void Console::Print(int color, fmt::string_view fmt, fmt::format_args args)
{
	Print(color, fmt::vformat(fmt, args));
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
	while (!first.empty() && first[0] == ' ')
		first = first.substr(1);
	auto second = std::string("[]");
	auto fullSec = std::string("");
	auto haveArgs = false;
	{
		auto space = first.find(' ');
		if (space != std::string::npos)
		{
			first = first.substr(0, space);
			//second = str.substr(space + 1);
			fullSec = str.substr(space + 1);
			//while (!second.empty() && second[0] == ' ')
			//	second = second.substr(1);
			second = quake2json(str.substr(space + 1));
			haveArgs = !second.empty();
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
				if ((cv.flags & CVar::Flags::Cheat) == CVar::Flags::Cheat && !cheatsEnabled)
				{
					Print(1, fmt::format("Changing {} is considered a cheat.", cv.name));
					return false;
				}
				while (true)
				{
					try
					{
						if (cv.Set(second))
						{
							Print(0, fmt::format("{} set to {}", cv.name, cv.ToString()));
							return true;
						}
						else
						{
							Print(1, fmt::format("Could not set cvar {} to {}", cv.name, second));
							return false;
						}
					}
					catch (std::runtime_error& x)
					{
						std::string what = x.what();
						if (what.find(" given array") != -1 && what.find(" entries, not ") != -1 && second.substr(0, 3) == "[ [")
						{
							//try again without the surrounding []
							second = second.substr(2, second.length() - 4);
							continue;
						}
						Print(1, what);
						return false;
					}
				}
			}
		}
	}
	for (auto& cc : ccmds)
	{
		if (cc.name == first)
		{
			if (cc.takesString)
			{
				auto a = json5pp::array({ fullSec });
				cc.act(a.as_array());
				return true;
			}

			try
			{
				cc.act(json5pp::parse5(second).as_array());
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
	{
		auto ca = calis.find(first);
		if (ca != calis.end())
		{
			commandBuffer = fmt::format("{}; {}", ca->second, commandBuffer);
		}
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
		commandBuffer = inputLine->value;
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

	if (!commandBuffer.empty())
	{
		if (waitTimer > 0)
		{
			waitTimer--;
			return false;
		}
		else
		{
			while (!commandBuffer.empty() && commandBuffer[0] == ' ')
				commandBuffer = commandBuffer.substr(1);

			auto sep = commandBuffer.find(';');
			if (sep != std::string::npos && commandBuffer.substr(0, 6) != "alias ")
			{
				//multiple commands left
				auto command = commandBuffer.substr(0, sep);
				commandBuffer = commandBuffer.substr(sep + 1);
				Execute(command);
			}
			else
			{
				auto command = commandBuffer;
				commandBuffer.clear();
				Execute(command);
			}
		}
		return false;
	}

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

	inputLine->rect = glm::vec4(12, offset.y + (h - 24), width - 8, offset.y + (h - 24) + 20);
	Sprite::DrawText(0, "]", glm::vec2(4, inputLine->rect.y));
	if (!prediction.empty())
		Sprite::DrawText(0, prediction, glm::vec2(inputLine->rect.x, inputLine->rect.y), glm::vec4(0.5f));
	inputLine->Draw(dt);
}

void Console::RegisterCVar(const std::string& name, CVar::Type type, void* target, CVar::Flags flags, int min, int max, CVarCallback onChange, const std::string& description)
{
	auto it = std::find_if(cvars.begin(), cvars.end(), [name](const auto& e)
	{
		return e.name == name;
	});
	if (it != cvars.end())
	{
		it->type = type;
		it->asVoid = target;
		it->flags = flags;
		it->onChange = onChange;
		it->description = description;
		return;
	}
	CVar cv;
	cv.name = name;
	cv.type = type;
	cv.asVoid = target;
	cv.flags = flags;
	cv.min = min;
	cv.max = max;
	cv.onChange = onChange;
	cv.description = description;
	cvars.push_back(cv);
}

void Console::RegisterCVar(const std::string& name, CVar::Type type, void* target, CVar::Flags flags, const std::string& description)
{
	RegisterCVar(name, type, target, flags, -1, -1, nullptr, description);
}

void Console::RegisterCVar(const std::string& name, CVar::Type type, void* target, const std::string& description)
{
	RegisterCVar(name, type, target, CVar::Flags::Normal, -1, -1, nullptr, description);
}

void Console::RegisterCCmd(const std::string& name, std::function<void(const jsonArray& args)> act, bool takesString, const std::string& description)
{
	auto it = std::find_if(ccmds.begin(), ccmds.end(), [name](const auto& e)
	{
		return e.name == name;
	});
	if (it != ccmds.end())
	{
		it->act = act;
		it->takesString = takesString;
		it->description = description;
		return;
	}
	CCmd cc;
	cc.name = name;
	cc.act = act;
	cc.takesString = takesString;
	cc.description = description;
	ccmds.push_back(cc);
}

void Console::LoadPersistentCVars(jsonObject& sets)
{
	for (auto& cv : cvars)
	{
		if ((cv.flags & CVar::Flags::Persistent) != CVar::Flags::Persistent)
			continue;
		if (sets[cv.name].is_null())
			continue;
		cv.Set(sets[cv.name].stringify());
	}
}

void Console::SavePersistentCVars(jsonObject& sets)
{
	for (auto& cv : cvars)
	{
		if ((cv.flags & CVar::Flags::Persistent) != CVar::Flags::Persistent)
			continue;
		switch (cv.type)
		{
		case CVar::Type::Bool: sets[cv.name] = *cv.asBool; break;
		case CVar::Type::Int: sets[cv.name] = *cv.asInt; break;
		case CVar::Type::Float: sets[cv.name] = *cv.asFloat; break;
		case CVar::Type::String: sets[cv.name] = *cv.asString; break;
		//case CVar::Type::Vec2: sets[cv.name] = *cv.asVec2; break;
		}
	}
}

bool CVar::Set(const std::string& value)
{
	if (value.empty())
		return false;

	//if (type == Type::String && value[0] != '\"')
	//	return Set(fmt::format("\"{}\"", value));
	//if ((type == Type::Vec2 || type == Type::Vec3 || type == Type::Vec4) && value[0] != '[')
	//	return Set(fmt::format("[{}]", value));
	if (type == Type::Color)
	{
		if (value[0] == '#')
			return Set(fmt::format("\"{}\"", value));
		if (value[0] != '[')
			return Set(fmt::format("[{}]", value));
	}

	auto json = json5pp::parse5(value);
	if (json.is_array())
	{
		auto oop = json.as_array()[0];
		json = json5pp::parse5(oop.stringify());
	}

	switch (type)
	{
	case Type::Bool:
		if (json.is_number())
			*asBool = json.as_integer() != 0;
		else if (json.is_boolean())
			*asBool = json.as_boolean();
		if (onChange) onChange(this);
		return true;
	case Type::Int:
		if (json.is_integer())
		{
			auto i = json.as_integer();
			if (!(min == -1 && max == -1))
				i = glm::clamp(i, min, max);
			*asInt = i;
		}
		if (onChange) onChange(this);
		return true;
	case Type::Float:
		if (json.is_number())
		{
			auto i = (float)json.as_number();
			if (!(min == -1 && max == -1))
				i = glm::clamp(i, (float)min, (float)max);
			*asFloat = i;
		}
		if (onChange) onChange(this);
		return true;
	case Type::String:
		if (json.is_number())
			*asString = fmt::format("{}", json.as_number());
		else if (json.is_string())
			*asString = json.as_string();
		if (onChange) onChange(this);
		return true;
	case Type::Vec2:
		if (json.is_array())
			*asVec2 = GetJSONVec2(json);
		if (onChange) onChange(this);
		return true;
	case Type::Vec3:
		if (json.is_array())
			*asVec3 = GetJSONVec3(json);
		if (onChange) onChange(this);
		return true;
	case Type::Vec4:
		if (json.is_array())
			*asVec4 = GetJSONVec4(json);
		if (onChange) onChange(this);
		return true;
	case Type::Color:
		*asVec4 = GetJSONColor(json);
		if (onChange) onChange(this);
		return true;
	}
	return false;
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

static void CCmdPrint(const jsonArray& args)
{
	if (args.size() < 1 && !args[0].is_string())
	{
		conprint(0, "Input value must be a string.");
		return;
	}
	if (args.size() == 2 && args[0].is_integer() && args[1].is_string())
	{
		conprint(args[0].as_integer(), "{}", args[1].as_string());
		return;
	}
	conprint(0, "{}", args[0].as_string());
}

static void CCmdWait(const jsonArray& args)
{
	if (args.size() == 0 || !args[0].is_integer())
	{
		conprint(0, "Input value must be an integer.");
		return;
	}
	console->waitTimer = args[0].as_integer();
}

static void CCmdAlias(const jsonArray& args)
{
	if (args.size() == 0)
	{
		for (const auto& a : console->calis)
		{
			conprint(0, "  {}: {}", a.first, a.second);
		}
		return;
	}
	else
	{
		if (!args[0].is_string())
		{
			conprint(1, "Alias name must be a string.");
			return;
		}
		if (args.size() > 1 && !args[1].is_string())
		{
			conprint(1, "Alias content must be string.");
			return;
		}
	}

	auto key = args[0].as_string();
	auto it = console->calis.find(key);
	if (args.size() == 1)
	{
		console->calis.erase(it);
		return;
	}
	console->calis[key] = args[1].as_string();
}

static void CCmdHelp(const jsonArray& args)
{
	if (args.size() < 1 || !args[0].is_string())
	{
		conprint(0, "Input value must be a string.");
		return;
	}
	auto target = args[0].as_string();
	auto cv = std::find_if(console->cvars.begin(), console->cvars.end(), [target](const auto& e)
	{
		return e.name == target;
	});
	if (cv != console->cvars.end())
	{
		if (cv->description.empty())
			conprint(0, "No help available.");
		else
			conprint(0, cv->description);
		//TODO: give more information.
		switch (cv->type)
		{
		case CVar::Type::Int:
		case CVar::Type::Float:
			conprint(0, "* {}", cv->type == CVar::Type::Int ? "Integer" : "Float");
			if (!(cv->min == -1 && cv->max == -1))
				conprint(0, "* Range: {} to {}", cv->min, cv->max);
			break;
		case CVar::Type::String: conprint(0, "* String"); break;
		case CVar::Type::Vec2: conprint(0, "* Vector (X, Y)"); break;
		case CVar::Type::Vec3: conprint(0, "* Vector (X, Y, Z)"); break;
		case CVar::Type::Vec4: conprint(0, "* Vector (X, Y, Z, W)"); break;
		case CVar::Type::Color: conprint(0, "* Color"); break;
		case CVar::Type::Bool: conprint(0, "* Boolean"); break;
		}
		if (cv->onChange) conprint(0, "* Has callback");
		if ((cv->flags & 1) == 1) conprint(1, "* This is a cheat");
		if ((cv->flags & 2) == 2) conprint(1, "* Persistent");
		return;
	}
	auto cc = std::find_if(console->ccmds.begin(), console->ccmds.end(), [target](const auto& e)
	{
		return e.name == target;
	});
	if (cc != console->ccmds.end())
	{
		if (cc->description.empty())
			conprint(0, "No help available.");
		else
			conprint(0, cc->description);
	}
}

static void CCmdVersion(const jsonArray& args)
{
	(void)(args);
	conprint(8, BECKETT_GAMENAME " - " BECKETT_VERSIONJOKE);
#ifdef DEBUG
	conprint(7, "Debug version: " __DATE__);
#endif
#ifdef _MSC_VER
	conprint(7, "Microsoft Visual C++ version {}.{}.", (_MSC_VER / 100), (_MSC_VER % 100));
#endif
	conprint(7, "OpenGL: {}", std::string((char*)glGetString(GL_VERSION)));
	conprint(7, "Vendor: {}", std::string((char*)glGetString(GL_VENDOR)));
	conprint(7, "GLSL {}", std::string((char*)glGetString(GL_SHADING_LANGUAGE_VERSION)));
	conprint(7, "Renderer: {}", std::string((char*)glGetString(GL_RENDERER)));
	conprint(7, "GLFW: {}", glfwGetVersionString());
#ifndef BECKETT_NO3DMODELS
	conprint(7, "UFBX: {}.{}.{}", ufbx_version_major(UFBX_VERSION), ufbx_version_minor(UFBX_VERSION), ufbx_version_patch(UFBX_VERSION));
#endif
	conprint(7, "MiniZip: " MZ_VERSION);
	conprint(7, "SoLoud: {}", SOLOUD_VERSION);
	conprint(7, "STB_Image: 2.30");
	conprint(7, "STB_Image_Write: 1.16");
	conprint(7, "STB_TrueType: 1.26");
#ifdef DEBUG
	conprint(7, "ImGUI version: " IMGUI_VERSION);
#endif
}

static void CCmdCVarList(const jsonArray& args)
{
	if (args.size() != 0 && !args[0].is_string())
	{
		conprint(0, "Pattern must be a string.");
		return;
	}
	size_t results = 0;

	for (const auto& cv : console->cvars)
	{
		if (args.size() == 0 || checkSplat(args[0].as_string(), cv.name))
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

static void CCmdCmdList(const jsonArray& args)
{
	if (args.size() != 0 && !args[0].is_string())
	{
		conprint(0, "Pattern must be a string.");
		return;
	}
	size_t results = 0;
	for (const auto& cc : console->ccmds)
	{
		if (args.size() == 0 || checkSplat(args[0].as_string(), cc.name))
		{
			conprint(0, "{}", cc.name);
			results++;
		}
	}
	conprint(0, "{} ccmds", results);
}

static void CCmdCRC32(const jsonArray& args)
{
	if (args.size() == 0 || !args[0].is_string())
	{
		conprint(0, "Input value must be a string.");
		return;
	}
	conprint(0, "{:X}", GetCRC(args[0].as_string()));
}
