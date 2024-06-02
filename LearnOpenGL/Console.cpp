#include <regex>

#include "Console.h"
#include "InputsMap.h"
#include "DialogueBox.h"

Console::Console()
{
	visible = true;
	Print(3, "Project Special K console");
	Print(3, "-------------------------");
	Print(7, "TODO: allow formatted printing, by which I mean fmt::format style.");

	inputLine.rect = glm::vec4(16, (height / 3) - 24, width - 8, 20);
	inputLine.font = 0;
	inputLine.Clear();

	history.clear();
	history.push_back("first");
	history.push_back("second");
	history.push_back("turd");
	historyCursor = 0;

	Sol.open_libraries(sol::lib::coroutine);
	Sol["dialogue"] = sol::yielding([&](sol::variadic_args va)
	{
		int style = 0;
		std::string line;
		switch (va.size())
		{
		case 0:
			line = "[[Forgot to specify a line]]";
			break;
		case 1:
			line = va[0].as<std::string>();
			break;
		case 2:
			line = va[0].as<std::string>();
			if (va[1].is<int>())
				style = va[1].as<int>();
			//else va[1] is the speaker and va[2] is a style
			break;
		}

		//do that mutex thing
		dlgBox->Text(line, style);
		
		//ensure we can see the result
		visible = false;
	});
}

void Console::Print(int color, const std::string& str)
{
	buffer.emplace_back(std::make_pair(clamp(color, 0, 8), str));
}

void Console::Print(const std::string& str)
{
	Print(0, str);
}

bool Console::Execute(const std::string& str)
{
	//auto ret = Sol.script("function __console()\n\n" + str + "\n\nend");
	//sol::coroutine __console = Sol["__console"];
	//__console();
	//sol::unsafe_function_result ret;
	try
	{
		Sol.script(str);
	}
	catch (sol::error e)
	{
		auto what = std::string(e.what());		
		auto isItYield = what == "lua: error: attempt to yield from outside a coroutine";
		if (!isItYield)
			Print(1, fmt::format("Error: {}", what));
	}
	return false;
}

bool Console::Character(unsigned int codepoint)
{
	return inputLine.Character(codepoint);
}

void Console::Tick(double dt)
{
	if (!visible)
		return;
	if (Inputs.Enter)
	{
		Inputs.Enter = false;
		history.emplace_back(inputLine.value);
		historyCursor = 0;
		Execute(inputLine.value);
		inputLine.Clear();
	}
	else if (Inputs.Up)
	{
		Inputs.Up = false;
		if (historyCursor < history.size())
		{
			historyCursor++;
			inputLine.Set(history[history.size() - historyCursor]);
		}
	}
	else if (Inputs.Down)
	{
		Inputs.Down = false;
		if (historyCursor > 1)
		{
			historyCursor--;
			inputLine.Set(history[history.size() - historyCursor]);
		}
		else
		{
			historyCursor = 0;
			inputLine.Clear();
		}
	}
	inputLine.Tick(dt);
}

void Console::Draw(double dt)
{
	static glm::vec4 colors[] =
	{
		glm::vec4(1, 1, 1, 1),
		glm::vec4(1, 0, 0, 1),
		glm::vec4(0, 1, 0, 1),
		glm::vec4(1, 1, 0, 1),
		glm::vec4(0, 0, 1, 1),
		glm::vec4(1, 0, 1, 1),
		glm::vec4(0, 1, 1, 1),
		glm::vec4(0.75, 0.75, 0.75, 1),
		glm::vec4(0.5, 0.5, 0.5, 1),
	};

	if (!visible)
		return;

	sprender->DrawSprite(whiteRect, glm::vec2(0), glm::vec2(width, height / 3), glm::vec4(0), 0, glm::vec4(0, 0, 0, 0.8), 0);

	auto pos = glm::vec2(4, (height / 3) - 42);
	for (unsigned int i = (unsigned int)buffer.size(), lines = 21; i-- > 0 && lines-- > 0;)
	{
		sprender->DrawText(0, buffer[i].second, pos, colors[buffer[i].first], 100.0f, 0, true);
		pos.y -= 16;
	}

	sprender->DrawText(0, "]", glm::vec2(4, inputLine.rect.y));
	inputLine.Draw(dt);
}
