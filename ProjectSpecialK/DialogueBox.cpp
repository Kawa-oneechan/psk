#include "DialogueBox.h"
#include "PanelLayout.h"
#include "InputsMap.h"

void DialogueBox::msbtStr(MSBTParams)
{
	if (tags.size() < 2)
	{
		conprint(2, "Missing paramater in MSBT Str: {}", toDisplay.substr(start, len));
		return;
	}
	if (tags[1] == "...")
		toDisplay.replace(start, len, TextGet("str:fix:001"));
	else if (tags[1] == "player")
		toDisplay.replace(start, len, thePlayer.Name);
	else if (tags[1] == "kun")
		toDisplay.replace(start, len, TextGet("str:kun"));
}

void DialogueBox::msbtEllipses(MSBTParams)
{
	tags;
	auto fakeTags = std::vector<std::string>
	{
		"str", "..."
	};
	msbtStr(fakeTags, start, len);
}

void DialogueBox::msbtDelay(MSBTParams)
{
	len; start;
	if (tags.size() < 2)
	{
		conprint(2, "Missing paramater in MSBT Delay: {}", toDisplay.substr(start, len));
		return;
	}
	//TODO: add checks for per-personality delays
	delay = (float)std::stoi(tags[1]);
}

void DialogueBox::msbtEmote(MSBTParams)
{
	tags; len; start;
	//TODO LATER
}

void DialogueBox::msbtBreak(MSBTParams)
{
	tags; len; start;
	state = State::WaitingForKey;
}

void DialogueBox::msbtClear(MSBTParams)
{
	tags; len; start;
	displayed.clear();
}

void DialogueBox::msbtEnd(MSBTParams)
{
	tags; len; start;
	state = State::Closing;
}
void DialogueBox::msbtPass(MSBTParams)
{
	tags;
	displayed += toDisplay.substr(start, len);
}

DialogueBox::DialogueBox()
{
	//Text(u8"Truth is... <color:1>the game</color> was rigged\nfrom the start.", 0, "Isabelle", glm::vec4(1, 0.98f, 0.56f, 1), glm::vec4(0.96f, 0.67f, 0.05f, 1));
	//Text(u8"Truth is... <color:1>the game</color> was rigged from the start.",
	//Text("Are you <color:3><str:player></color>? <delay:1000>Hiii! Welcome to <color:2>Project Special K</color>!",
	Text(TextGet("dlg:sza:wack"),
		Database::Find<Villager>("psk:cat00", villagers));

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
		}
	}
}

void DialogueBox::Wrap()
{
	size_t lastSpace = 0xFFFF;
	auto threeLines = sprender->MeasureText(font, "1\n2\n3\n", 100).y;
	for (size_t i = 0; i < toDisplay.length();)
	{
		unsigned int ch;
		size_t size;
		std::tie(ch, size) = GetChar(toDisplay, i);

		if (size == 1 && (std::isblank(ch) || ch == '\n'))
			lastSpace = i;

		//Always break if the current character is ideographic.
		//Massive hack, absolutely breaks Kinsoku Shori rules. I don't care.
		if (size == 3 && (ch >= 0x2E80 && ch < 0xF000))
			lastSpace = i;
		auto space = sprender->MeasureText(font, toDisplay.substr(0, i + size), 100);
		if (space.x > 650)
		{
			if (lastSpace == 0xFFFF)
			{
				lastSpace = i;
				toDisplay.insert(toDisplay.begin() + i, '\n');
				i++;
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
		if (space.y >= threeLines)
		{
			toDisplay[lastSpace] = '>';
			toDisplay.insert(lastSpace, "<break");
		}

		i += size;
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

	if (state == State::Done)
		state = State::Opening;
	else
		state = State::Writing;
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

void DialogueBox::Text(const std::string& text, VillagerP speaker)
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

void DialogueBox::Draw(float dt)
{
	if (state == State::Done)
		return;

	time += dt * wobbleTimeScale;

	auto dlgScale = scale;

	if (state == State::Opening || state == State::Closing)
		dlgScale *= glm::mix(0.0f, 1.0f, tween);

	auto dlgWidth = bubble[0].width * dlgScale;
	auto dlgHeight = bubble[0].height * dlgScale;
	auto dlgLeft = (int)(width / 2) - dlgWidth;
	auto dlgTop = (int)height - bubble[0].height - 16;

	wobble.Use();
	gradient[0].Use(1);
	gradient[1].Use(2);
	wobble.SetInt("gradient1", 1);
	wobble.SetInt("gradient2", 2);
	wobble.SetFloat("time", time);

	sprender->DrawSprite(&wobble, bubble[bubbleNum], glm::vec2(dlgLeft, dlgTop), glm::vec2(dlgWidth * 2, dlgHeight), glm::vec4(0, 0, bubble[bubbleNum].width * 2, bubble[bubbleNum].height), 0, bubbleColor);

	sprender->DrawText(font, displayed, glm::vec2(dlgLeft + (200 * dlgScale), dlgTop + (100 * dlgScale)), textColor, 150 * dlgScale);

	if (!name.empty())
	{
		const auto tagAngle = -2.0f;
		const auto tagPos = glm::vec2((int)(width / 2) - bubble[0].width + (150 * scale), dlgTop + (sinf(time * 2) * 10) * scale);
		const auto tagSize = glm::vec2(nametag[0].z, nametag[0].w) * scale;
		const auto alpha = glm::clamp((tween * 2.0f) - 0.75f, 0.0f, 1.0f);
		nametagColor[0].a = alpha;
		nametagColor[1].a = alpha;

		const auto tagPosL = tagPos;
		const auto tagPosM = tagPosL + glm::vec2(cosf(glm::radians(tagAngle)) * tagSize.x, sinf(glm::radians(tagAngle)) * tagSize.x);
		const auto tagPosR = tagPosM + glm::vec2(cosf(glm::radians(tagAngle)) * nametagWidth, sinf(glm::radians(tagAngle)) * nametagWidth);
		//TODO: do proper "rotate point around origin point" thing.
		const auto tagPosT = tagPosL + glm::vec2(cosf(glm::radians(tagAngle)) * (tagSize.x - 16), sinf(glm::radians(tagAngle)) * (tagSize.x - 512));
		sprender->DrawSprite(nametag, tagPosL, tagSize, nametag[0], tagAngle, nametagColor[0], TopLeft);
		sprender->DrawSprite(nametag, tagPosM, glm::vec2(nametagWidth, tagSize.y), nametag[2], tagAngle, nametagColor[0], TopLeft);
		sprender->DrawSprite(nametag, tagPosR, tagSize, nametag[1], tagAngle, nametagColor[0], TopLeft);
		sprender->DrawText(1, name, tagPosT, nametagColor[1], 120 * scale, tagAngle);
	}

	if (state == State::WaitingForKey)
	{
		auto arr = (*UI::controls)[6];
		sprender->DrawSprite(*UI::controls, glm::vec2((width / 2) - (arr.z / 2), height - arr.w - 20), glm::vec2(arr.z, arr.w), arr, 0.0f, UI::themeColors["primary"]);
	}

	//maybe afterwards port this to the UI Panel system?
}

void DialogueBox::Tick(float dt)
{
	if (state == State::Done)
	{
		tween = 0;
	}

	if (state == State::Opening)
	{
		tween += dt * tweenTimeScale;
		if (tween >= 1.0f)
			state = State::Writing;
	}

	if (state == State::Writing)
	{
		tween = 1;

		delay -= dt;
		if (delay > 0)
			return;

		if (displayCursor >= toDisplay.length())
		{
			state = State::WaitingForKey;
		}

		delay = 0;

		unsigned int ch;
		size_t size;
		std::tie(ch, size) = GetChar(toDisplay, displayCursor);
		displayCursor += size;

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
			AppendChar(displayed, ch);

			if (bubbleNum == 3 || Sound == Sound::Bebebese)
				bebebese->Play(true);
		}
		if (delay < glm::epsilon<float>())
			delay = 50;
	}

	if (state == State::WaitingForKey)
	{
		if (Inputs.Enter)
		{
			Inputs.Enter = false;

			if (displayCursor >= toDisplay.length())
			{
				state = State::Closing;
				tween = 1;
				if (mutex != nullptr)
				{
					*mutex = false;
					mutex = nullptr;
				}
			}
			else
			{
				displayed.clear();
				state = State::Writing;
			}
		}
	}

	if (state == State::Closing)
	{
		tween -= dt * tweenTimeScale;
		if (tween <= 0.0f)
			state = State::Done;
	}
}
