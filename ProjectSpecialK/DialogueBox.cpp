#include "DialogueBox.h"

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

	displayCursor = 0;
	time = 0;
	delay = 0;

	//Text(u8"Truth is... <color:1>the game</color> was rigged\nfrom the start.", 0, "Isabelle", glm::vec4(1, 0.98f, 0.56f, 1), glm::vec4(0.96f, 0.67f, 0.05f, 1));
	Text(u8"Truth is... <color:1>the game</color> was rigged\nfrom the start.",
		(Villager*)Database::Find<Villager>("psk:kaw", &villagers));
}

void DialogueBox::Text(const std::string& text)
{
	displayed.clear();
	toDisplay = text;
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

	//maybe afterwards port this to the UI Panel system?

	//sprender->DrawText(1, fmt::format("DialogueBox: {}", time), glm::vec2(0, 16), glm::vec4(0, 0, 0, 0.25), 50);
}

void DialogueBox::Tick(double dt)
{
	delay -= (float)dt;
	if (delay > 0)
		return;

	if (displayCursor >= toDisplay.length())
		return;

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
		auto func = msbtPhase3.find(msbt[0]);
		if (func != msbtPhase3.end())
		{
			std::invoke(func->second, this, msbt, (int)msbtStart - 1, (int)(msbtEnd - msbtStart) + 2);
			//func->second(msbt, (int)msbtStart - 1, (int)(msbtEnd - msbtStart) + 2);
		}
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
	}
	delay = 50;
}

