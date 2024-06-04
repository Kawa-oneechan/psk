#include <regex>

#include "Console.h"
#include "InputsMap.h"
#include "DialogueBox.h"

Console::Console()
{
	visible = false;
	Print(3, "Project Special K");
	Print(3, "-----------------");

	inputLine = new TextField();
	
	auto il = (TextField*)inputLine;
	il->rect = glm::vec4(16, (height / 3) - 24, width - 8, 20);
	il->font = 0;
	il->Clear();

	history.clear();
	historyCursor = 0;
	scrollCursor = 0;

	Sol.open_libraries(sol::lib::coroutine);

	Sol["print"] = [&](sol::variadic_args va)
	{
		Print(va[0]);
	};

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
	try
	{
		//sol::coroutine s = Sol.load(str);
		//s();
		Sol.script(str);
	}
	catch (sol::error& e)
	{
		std::string what = e.what();
		if (what.find("attempt to yield from outside a coroutine") != -1)
			; //Do nothing. Accept this silently.
		else if (what.find("[string \"") != -1)
			Execute("print(" + str + ")");
		else
			Print(1, fmt::format("Error: {}", what));
	}
	return false;
}

bool Console::Character(unsigned int codepoint)
{
	return ((TextField*)inputLine)->Character(codepoint);
}

void Console::Tick(double dt)
{
	if (!visible)
		return;

	auto il = (TextField*)inputLine;

	if (Inputs.Enter)
	{
		Inputs.Enter = false;
		if (history.size() == 0 || history.back() != il->value)
			history.emplace_back(il->value);
		historyCursor = 0;
		Execute(il->value);
		il->Clear();
	}
	else if (Inputs.Up)
	{
		Inputs.Up = false;
		if (historyCursor < history.size())
		{
			historyCursor++;
			il->Set(history[history.size() - historyCursor]);
		}
	}
	else if (Inputs.Down)
	{
		Inputs.Down = false;
		if (historyCursor > 1)
		{
			historyCursor--;
			il->Set(history[history.size() - historyCursor]);
		}
		else
		{
			historyCursor = 0;
			il->Clear();
		}
	}
	else if (Inputs.PgUp)
	{
		Inputs.PgUp = false;
		if (scrollCursor < buffer.size())
		{
			scrollCursor += 5;
			if (scrollCursor >= buffer.size())
				scrollCursor = (int)buffer.size() - 1;
		}
	}
	else if (Inputs.PgDown)
	{
		Inputs.PgDown = false;
		if (scrollCursor > 0)
		{
			scrollCursor -= 5;
			if (scrollCursor < 0)
				scrollCursor = 0;
		}
	}
	il->Tick(dt);
}

void Console::Draw(double dt)
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

	sprender->DrawSprite(whiteRect, glm::vec2(0), glm::vec2(width, height / 3), glm::vec4(0), 0, glm::vec4(0, 0, 0, 0.8), 0);

	auto pos = glm::vec2(4, (height / 3) - 42);
	for (unsigned int i = (unsigned int)buffer.size() - scrollCursor, lines = 21; i-- > 0 && lines-- > 0;)
	{
		sprender->DrawText(0, buffer[i].second, pos, colors[buffer[i].first], 100.0f, 0, true);
		pos.y -= 16;
	}

	auto il = (TextField*)inputLine;
	sprender->DrawText(0, "]", glm::vec2(4, il->rect.y));
	il->Draw(dt);
}
