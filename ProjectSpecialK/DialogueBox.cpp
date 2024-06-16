#include "DialogueBox.h"
#include "InputsMap.h"

void DialogueBox::msbtStr(MSBTParams)
{
	if (tags[1] == "...")
		toDisplay.replace(start, len, TextGet("str:fix:001"));
	else if (tags[1] == "player")
		toDisplay.replace(start, len, thePlayer.Name);
	else if (tags[1] == "kun")
		toDisplay.replace(start, len, TextGet("str:kun"));
}

void DialogueBox::msbtEllipses(MSBTParams)
{
	auto fakeTags = std::vector<std::string>
	{
		"str", "..."
	};
	msbtStr(fakeTags, start, len);
}

void DialogueBox::msbtDelay(MSBTParams)
{
	delay = (float)std::stoi(tags[1]);
}

void DialogueBox::msbtEmote(MSBTParams)
{

}

void DialogueBox::msbtBreak(MSBTParams)
{
	state = DialogueBoxState::WaitingForKey;
}

void DialogueBox::msbtClear(MSBTParams)
{
	displayed.clear();
}

void DialogueBox::msbtEnd(MSBTParams)
{
	state = DialogueBoxState::Closing;
}
void DialogueBox::msbtPass(MSBTParams)
{
	displayed += toDisplay.substr(start, len);
}

DialogueBox::DialogueBox()
{
	bubble[0] = new Texture("ui/dialogue/dialogue.png", GL_MIRRORED_REPEAT);
	bubble[1] = new Texture("ui/dialogue/exclamation.png", GL_MIRRORED_REPEAT);
	bubble[2] = new Texture("ui/dialogue/dream.png", GL_MIRRORED_REPEAT);
	bubble[3] = new Texture("ui/dialogue/system.png", GL_MIRRORED_REPEAT);
	//bubble[4] = new Texture("ui/dialogue/wildworld.png", GL_REPEAT);
	gradient[0] = new Texture("gradient_thin.png");
	gradient[1] = new Texture("gradient_wide.png");
	nametag = new Texture("ui/dialogue/nametag.png");
	GetAtlas(nametagAtlas, "ui/dialogue/nametag.json");
	nametagWidth = 0;
	wobble = new Shader("shaders/wobble.fs");
	bebebese = new Audio("sound/animalese/base/Voice_Monology.wav");

	Sound = DialogueBoxSound::Bebebese;
	state = DialogueBoxState::Writing;
	
	displayCursor = 0;
	time = 0;
	delay = 0;

	//Text(u8"Truth is... <color:1>the game</color> was rigged\nfrom the start.", 0, "Isabelle", glm::vec4(1, 0.98f, 0.56f, 1), glm::vec4(0.96f, 0.67f, 0.05f, 1));
	//Text(u8"Truth is... <color:1>the game</color> was rigged from the start.",
	//Text("Are you <color:3><str:player></color>? <delay:1000>Hiii! Welcome to <color:2>Project Special K</color>!",
	Text(TextGet("dlg:sza:wack"),
		Database::Find<Villager>("psk:cat02", &villagers));

	//Text("I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I", 0);
	//Text("IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII", 0);
}

void DialogueBox::Preprocess()
{
	for (size_t i = 0; i < toDisplay.length(); i++)
	{
		auto msbtStart = toDisplay.find_first_of('<', i);
		if (msbtStart != std::string::npos)
		{
			msbtStart++;
			auto msbtEnd = toDisplay.find_first_of('>', msbtStart);
			i = msbtEnd;

			auto msbtWhole = toDisplay.substr(msbtStart, msbtEnd - msbtStart);
			auto msbt = Split(msbtWhole, ':');
			auto func = msbtPhase1.find(msbt[0]);
			if (func != msbtPhase1.end())
			{
				std::invoke(func->second, this, msbt, (int)msbtStart - 1, (int)(msbtEnd - msbtStart) + 2);
				i = msbtStart; //-1 because we may have subbed in a new tag.
			}
			//else
			//	conprint(1, "DialogueBox::Preprocess: don't know how to handle {}.", msbtWhole);
			//no need to report on that, whatever Preprocess can't handle, Tick and DrawString ought.
		}
	}
}

void DialogueBox::Wrap()
{
	size_t lastSpace = -1;
	for (size_t i = 0; i < toDisplay.length(); i++)
	{
		if (std::isblank(toDisplay[i]))
			lastSpace = i;
		auto width = sprender->MeasureText(font, toDisplay.substr(0, i), 100).x;
		if (width > 650)
		{
			if (lastSpace == -1)
			{
				lastSpace = i;
				toDisplay.insert(toDisplay.begin() + i, '\n');
				i++;
				//TODO: if we've reached three size 100 lines, add a <break> command instead.
			}
			else
			{
				if (std::isblank(toDisplay[lastSpace]))
					toDisplay[lastSpace] = '\n';
				else
				{
					toDisplay.insert(toDisplay.begin() + lastSpace, '\n');
					i++;
				}
			}
		}
	}
}

void DialogueBox::Text(const std::string& text)
{
	displayed.clear();
	toDisplay = text;

	Preprocess();
	Wrap();

	displayCursor = 0;
	delay = 50;
}

void DialogueBox::Text(const std::string& text, int style, const std::string& speaker, const glm::vec4& tagBack, const glm::vec4& tagInk)
{
	Style(style);
	if (style != 3)
	{
		name = speaker;
		nametagWidth = sprender->MeasureText(1, name, 120).x - 32;
		nametagColor[0] = tagBack;
		nametagColor[1] = tagInk;
	}
	else
	{
		name.clear();
	}
	Text(text);
}

void DialogueBox::Text(const std::string& text, int style)
{
	Style(style);
	Text(text);
}

void DialogueBox::Text(const std::string& text, Villager* speaker)
{
	Style(0);
	if (speaker != nullptr)
	{
		name = speaker->Name();
		nametagWidth = sprender->MeasureText(1, name, 120).x - 32;
		nametagColor[0] = speaker->NameTag[0];
		nametagColor[1] = speaker->NameTag[1];
	}
	Text(text);
}

void DialogueBox::Style(int style)
{
	style = clamp(style, 0, 3);

	if (style == 3) //system
	{
		bubbleColor = UI::themeColors["primary"];
		textColor = UI::textColors[8];
		font = 1;
		bubbleNum = 3;
	}
	else
	{
		bubbleColor = UI::themeColors["dialogue"];
		textColor = UI::textColors[0];
		font = 2;
		bubbleNum = style;
	}
}

void DialogueBox::Draw(double dt)
{
	if (state == DialogueBoxState::Done)
		return;

	time += (float)dt * 0.005f;

	auto dlgScale = scale;
	auto dlgWidth = bubble[0]->width * dlgScale;
	auto dlgHeight = bubble[0]->height * dlgScale;
	auto dlgLeft = (int)(width / 2) - dlgWidth;
	auto dlgTop = (int)height - dlgHeight - 10;

	wobble->Use();
	gradient[0]->Use(1);
	gradient[1]->Use(2);
	wobble->SetInt("gradient1", 1);
	wobble->SetInt("gradient2", 2);
	wobble->SetFloat("time", time);

	//if (bubbleNum == 4)
	//	sprender->DrawSprite(*bubble[bubbleNum], glm::vec2(dlgLeft, dlgTop), glm::vec2(dlgWidth * 2, dlgHeight), glm::vec4(0));
	//else
	{
		sprender->DrawSprite(wobble, bubble[bubbleNum], glm::vec2(dlgLeft, dlgTop), glm::vec2(dlgWidth * 2, dlgHeight), glm::vec4(0, 0, bubble[bubbleNum]->width * 2, bubble[bubbleNum]->height), 0, bubbleColor, 0);
		//sprender->DrawSprite(wobble, bubble[bubbleNum], glm::vec2(dlgLeft + dlgWidth, dlgTop), glm::vec2(dlgWidth, dlgHeight), glm::vec4(0, 0, bubble[0]->width, bubble[0]->height), 0, bubbleColor, 1);
	}

	sprender->DrawText(font, displayed, glm::vec2(dlgLeft + (200 * scale), dlgTop + (100 * scale)), textColor, 150 * scale);

	if (!name.empty())
	{
		const auto tagAngle = -2.0f;
		const auto tagPos = glm::vec2(dlgLeft + (150 * scale), dlgTop + (sinf(time * 2) * 10) * scale);
		const auto tagSize = glm::vec2(nametagAtlas[0].z, nametagAtlas[0].w) * scale;
		//const auto tagMidWidth = 128; //pre-measure this to fit in the Text() calls.

		const auto tagPosL = tagPos;
		const auto tagPosM = tagPosL + glm::vec2(cosf(glm::radians(tagAngle)) * tagSize.x, sinf(glm::radians(tagAngle)) * tagSize.x);
		const auto tagPosR = tagPosM + glm::vec2(cosf(glm::radians(tagAngle)) * nametagWidth, sinf(glm::radians(tagAngle)) * nametagWidth);
		//TODO: figure this one out properly
		const auto tagPosT = tagPosL + glm::vec2(cosf(glm::radians(tagAngle)) * (tagSize.x - 16), sinf(glm::radians(tagAngle)) * (tagSize.x - 512));
		sprender->DrawSprite(nametag, tagPosL, tagSize, nametagAtlas[0], tagAngle, nametagColor[0], SPR_TOPLEFT);
		sprender->DrawSprite(nametag, tagPosM, glm::vec2(nametagWidth, tagSize.y), nametagAtlas[2], tagAngle, nametagColor[0], SPR_TOPLEFT);
		sprender->DrawSprite(nametag, tagPosR, tagSize, nametagAtlas[1], tagAngle, nametagColor[0], SPR_TOPLEFT);
		sprender->DrawText(1, name, tagPosT, nametagColor[1], 120 * scale, tagAngle);
	}

	if (state == DialogueBoxState::WaitingForKey)
	{
		auto arr = UI::controlsAtlas[6];
		sprender->DrawSprite(UI::controls, glm::vec2((width / 2) - (arr.z / 2), height - arr.w - 20), glm::vec2(arr.z, arr.w), arr, 0.0f, UI::themeColors["primary"]);
	}

	//maybe afterwards port this to the UI Panel system?

	//sprender->DrawText(1, fmt::format("DialogueBox: {}", time), glm::vec2(0, 16), glm::vec4(0, 0, 0, 0.25), 50);
}

void DialogueBox::Tick(double dt)
{
	if (state == DialogueBoxState::Opening)
	{
		//TODO: wait for animation
		state = DialogueBoxState::Writing;
	}

	if (state == DialogueBoxState::Writing)
	{
		delay -= (float)dt;
		if (delay > 0)
			return;

		if (displayCursor >= toDisplay.length())
		{
			state = DialogueBoxState::WaitingForKey;
		}

		//reset delay
		delay = 0;

		//We use UTF-8 to store strings but display in UTF-16.
		unsigned int ch = toDisplay[displayCursor++] & 0xFF;
		if ((ch & 0xE0) == 0xC0)
		{
			ch = (ch & 0x1F) << 6;
			ch |= (toDisplay[displayCursor++] & 0x3F);
		}
		else if ((ch & 0xF0) == 0xE0)
		{
			ch = (ch & 0x1F) << 12;
			ch |= (toDisplay[displayCursor++] & 0x3F) << 6;
			ch |= (toDisplay[displayCursor++] & 0x3F);
		}

		if (ch == '<')
		{
			auto msbtEnd = toDisplay.find_first_of('>', displayCursor);
			if (msbtEnd == -1) goto displayIt;
			auto msbtStart = displayCursor;
			displayCursor = msbtEnd + 1;

			auto msbtWhole = toDisplay.substr(msbtStart, msbtEnd - msbtStart);
			auto msbt = Split(msbtWhole, ':');
			auto func = msbtPhase2.find(msbt[0]);
			if (func != msbtPhase2.end())
			{
				std::invoke(func->second, this, msbt, (int)msbtStart - 1, (int)(msbtEnd - msbtStart) + 2);
			}
			else
				conprint(1, "DialogueBox::Tick: don't know how to handle {}.", msbtWhole);
		}
		else
		{
		displayIt:
			//Gotta re-encode the UTF-16 to UTF-8 here.
			if (ch < 0x80)
				displayed += ch;
			else if (ch < 0x0800)
			{
				displayed += (char)(((ch >> 6) & 0x1F) | 0xC0);
				displayed += (char)(((ch >> 0) & 0x3F) | 0x80);
			}
			else if (ch < 0x10000)
			{
				displayed += (char)(((ch >> 12) & 0x0F) | 0xE0);
				displayed += (char)(((ch >> 6) & 0x3F) | 0x80);
				displayed += (char)(((ch >> 0) & 0x3F) | 0x80);
			}

			if (bubbleNum == 3 || Sound == DialogueBoxSound::Bebebese)
				bebebese->Play(true);
		}
		if (delay < glm::epsilon<float>())
			delay = 50;
	}

	if (state == DialogueBoxState::WaitingForKey)
	{
		if (Inputs.Enter)
		{
			Inputs.Enter = false;

			if (displayCursor >= toDisplay.length())
			{
				state = DialogueBoxState::Closing;
				if (mutex != nullptr)
				{
					*mutex = false;
					mutex = nullptr;
				}
			}
			else
			{
				displayed.clear();
				state = DialogueBoxState::Writing;
			}
		}
	}

	if (state == DialogueBoxState::Closing)
	{
		//TODO: wait for animation
		state = DialogueBoxState::Done;
	}
}

