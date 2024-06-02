#include "Console.h"
#include "InputsMap.h"
#include "DialogueBox.h"

Console::Console()
{
	visible = true;
	Print("Project Special K console");
	Print("-------------------------");
	Print("TODO: allow formatted printing.");
	Print("lololol LOL!");

	inputLine.rect = glm::vec4(16, (height / 3) - 24, width - 8, 20);
	inputLine.font = 0;
	inputLine.value = "dialogue(\"Can we do <color:2>colors</color>?\")"; //"dialogue(\"Test...\")";

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

void Console::Print(const std::string& str)
{
	buffer.emplace_back(str);
}

bool Console::Execute(const std::string& str)
{
	auto ret = Sol.script("function __console()\n\n" + str + "\n\nend");
	sol::coroutine __console = Sol["__console"];
	__console();
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
		Execute(inputLine.value);
		//TODO: give the TextField itself a clear().
		inputLine.value.clear();
		inputLine.caret = 0;
	}
	inputLine.Tick(dt);
}

void Console::Draw(double dt)
{
	if (!visible)
		return;

	sprender->DrawSprite(whiteRect, glm::vec2(0), glm::vec2(width, height / 3), glm::vec4(0), 0, glm::vec4(0, 0, 0, 0.8), 0);

	auto pos = glm::vec2(4, (height / 3) - 42);
	for (unsigned int i = (unsigned int)buffer.size(), lines = 21; i-- > 0 && lines-- > 0;)
	{
		sprender->DrawText(0, buffer[i], pos);
		pos.y -= 16;
	}

	sprender->DrawText(0, "]", glm::vec2(4, inputLine.rect.y));
	inputLine.Draw(dt);
}
