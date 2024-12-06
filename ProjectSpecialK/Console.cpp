#include <regex>

#include "Console.h"
#include "TextField.h"
#include "InputsMap.h"
#include "DialogueBox.h"
#include "PanelLayout.h"

extern float timeScale;
extern bool debugPanelLayoutPolygons;
extern bool debuggerEnabled;
extern bool cheatsEnabled;

Console::Console()
{
	visible = false;
	
	hardcopy = std::ofstream("console.log", std::ios::trunc);

	Print(3, "Project Special K");
	Print(3, "-----------------");

	inputLine = new TextField();
	//inputLine->rect = glm::vec4(16, (height / 3) - 24, width - 8, 20);
	inputLine->font = 0;
	inputLine->Clear();

	history.clear();
	historyCursor = 0;
	scrollCursor = 0;

	timer = 0.0f;
	appearState = 0;

	RegisterCVar("timescale", CVar::Type::Float, &timeScale);
	RegisterCVar("playername", CVar::Type::String, &thePlayer.Name);
	RegisterCVar("playerbells", CVar::Type::Int, &thePlayer.Bells, true);
	RegisterCVar("sv_cheats", CVar::Type::Bool, &cheatsEnabled);
	RegisterCVar("r_polygons", CVar::Type::Bool, &debugPanelLayoutPolygons);
#ifdef DEBUG
	RegisterCVar("debugger", CVar::Type::Bool, &debuggerEnabled, true);
#endif
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

	buffer.emplace_back(std::make_pair(clamp(color, 0, 8), str));
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

bool Console::Execute(const std::string& str)
{
	auto split = Split((std::string&)str, ' ');
	if (split.size() >= 1)
	for (auto& cv : cvars)
	{
		if (cv.name == split[0])
		{
			if (split.size() == 1)
			{
				switch (cv.type)
				{
				case CVar::Type::Bool: Print(0, fmt::format("{} is {}", cv.name, *cv.asBool)); return true;
				case CVar::Type::Int: Print(0, fmt::format("{} is {}", cv.name, *cv.asInt)); return true;
				case CVar::Type::Float: Print(0, fmt::format("{} is {}", cv.name, *cv.asFloat)); return true;
				case CVar::Type::String: Print(0, fmt::format("{} is \"{}\"", cv.name, *cv.asString)); return true;
				case CVar::Type::Vec2: Print(0, fmt::format("{} is [{}, {}]", cv.name, cv.asVec2->x, cv.asVec2->y)); return true;
				case CVar::Type::Vec3: Print(0, fmt::format("{} is [{}, {}, {}]", cv.name, cv.asVec3->x, cv.asVec3->y, cv.asVec3->z)); return true;
				case CVar::Type::Color:
				case CVar::Type::Vec4: Print(0, fmt::format("{} is [{}, {}, {}, {}]", cv.name, cv.asVec4->x, cv.asVec4->y, cv.asVec4->z, cv.asVec4->w)); return true;
				}
			}
			else
			{
				auto second = str.substr(str.find(' '));
				if (cv.cheat && !cheatsEnabled)
				{
					Print(1, fmt::format("Changing {} is considered a cheat.", cv.name));
					return false;
				}
				if (cv.Set(second))
				{
					switch (cv.type)
					{
					case CVar::Type::Bool: Print(0, fmt::format("{} set to {}", cv.name, *cv.asBool)); return true;
					case CVar::Type::Int: Print(0, fmt::format("{} set to {}", cv.name, *cv.asInt)); return true;
					case CVar::Type::Float: Print(0, fmt::format("{} set to {}", cv.name, *cv.asFloat)); return true;
					case CVar::Type::String: Print(0, fmt::format("{} set to \"{}\"", cv.name, *cv.asString)); return true;
					case CVar::Type::Vec2: Print(0, fmt::format("{} set to [{}, {}]", cv.name, cv.asVec2->x, cv.asVec2->y)); return true;
					case CVar::Type::Vec3: Print(0, fmt::format("{} set to [{}, {}, {}]", cv.name, cv.asVec3->x, cv.asVec3->y, cv.asVec3->z)); return true;
					case CVar::Type::Color:
					case CVar::Type::Vec4: Print(0, fmt::format("{} set to [{}, {}, {}, {}]", cv.name, cv.asVec4->x, cv.asVec4->y, cv.asVec4->z, cv.asVec4->w)); return true;
					}
				}
				else
				{
					Print(2, fmt::format("Could not set cvar {} to {}", cv.name, second));
					return false;
				}
				return true;
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
			; //Do nothing. Accept this silently.
		else
			Print(1, fmt::format("Error: {}", what));
	}
	return false;
}

bool Console::Character(unsigned int codepoint)
{
	return inputLine->Character(codepoint);
}

bool Console::Scancode(unsigned int scancode)
{
	if (scancode == 28) //enter
	{
		if (history.size() == 0 || history.back() != inputLine->value)
			history.emplace_back(inputLine->value);
		historyCursor = 0;
		Execute(inputLine->value);
		inputLine->Clear();
		return true;
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
	//TODO: tab completion. See ImGUI console demo for details.
	return inputLine->Scancode(scancode);
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

void Console::Tick(float dt)
{
	if (!visible)
		return;

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
	timer = clamp(timer, 0.0f, 1.0f);

	inputLine->Tick(dt);
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
		offset.y += glm::mix(-h, 0.0f, glm::linearInterpolation(timer));

	Sprite::DrawSprite(*whiteRect, offset, glm::vec2(width, h), glm::vec4(0), 0, glm::vec4(0, 0, 0, 0.8));

	auto pos = offset + glm::vec2(4, h - 42);
	for (unsigned int i = (unsigned int)buffer.size() - scrollCursor, lines = 21; i-- > 0 && lines-- > 0;)
	{
		Sprite::DrawText(0, buffer[i].second, pos, colors[buffer[i].first], 100.0f, 0, true);
		pos.y -= 15;
	}

	inputLine->rect = glm::vec4(16, offset.y + (h - 24), width - 8, offset.y + (h - 24) + 20);
	Sprite::DrawText(0, "]", glm::vec2(4, inputLine->rect.y));
	inputLine->Draw(dt);
}

void Console::RegisterCVar(const std::string& name, CVar::Type type, void* target, bool cheat)
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
	cvars.push_back(cv);
}
