#include "DialogueBox.h"
#include "PanelLayout.h"
#include "InputsMap.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "support/glm/gtx/rotate_vector.hpp"

void DialogueBox::msbtStr(MSBTParams)
{
	if (tags.size() < 2)
	{
		conprint(2, "Missing parameter in MSBT Str: {}", toDisplay.substr(start, len));
		return;
	}
	if (tags[1] == "...")
		toDisplay.replace(start, len, TextGet("str:fix:001"));
	else if (tags[1] == "player")
		toDisplay.replace(start, len, thePlayer.Name);
	else if (tags[1] == "kun")
		toDisplay.replace(start, len, TextGet("str:kun"));
	else if (tags[1] == "vname")
	{
		if (speaker == nullptr)
		{
			conprint(2, "MSBT Str called for villager name, but no speaker is set.");
			toDisplay.replace(start, len, "Buggsy");
			return;
		}
		toDisplay.replace(start, len, speaker->Name());
	}
	else if (tags[1] == "vspec")
	{
		if (speaker == nullptr)
		{
			conprint(2, "MSBT Str called for villager species, but no speaker is set.");
			toDisplay.replace(start, len, "bug");
			return;
		}
		toDisplay.replace(start, len, speaker->Species());
	}
	else if (tags[1] == "catchphrase")
	{
		if (speaker == nullptr)
		{
			conprint(2, "MSBT Str called for villager catchphrase, but no speaker is set.");
			toDisplay.replace(start, len, "bugbug");
			return;
		}
		toDisplay.replace(start, len, speaker->Catchphrase());
	}
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
		conprint(2, "Missing parameter in MSBT Delay: {}", toDisplay.substr(start, len));
		return;
	}
	else if (tags.size() == 2)
	{
		delay = (float)std::stoi(tags[1]);
	}
	else
	{
		if (speaker == nullptr)
		{
			conprint(2, "MSBT Delay called with personality dependency, but no speaker is set.");
			return;
		}
		/*
		Function 198.0 delayPersonality takes eight separate values.
		If tags.size() == 9, we *may* have such a setup (0 being the "delay" tag itself).
		But if we want to support *arbitrary* personalities, we need a way to specify
		which ones.

		The solution is obvious: "<delay:10:10:20:20:10:20:10:20>" is like above, but
		"<delay:default:10:uchi:20:horny:40>" has a clear difference in that the first
		argument is NOT A NUMBER.

		So if tags.size() == 9 && std::isdigit(tags[1][0]) we have an 198.0 style delay,
		but otherwise we have ID-delay pairs.

		If the current speaker's personality is not listed, see if their personality's
		base fallback is and use that instead.
		The "default" delay, if given, will apply if the speaker's personality is not
		listed (even after falling back). If it's not and the speaker's personality
		isn't listed, we default the default to 50.
		*/
	}
}

void DialogueBox::msbtEmote(MSBTParams)
{
	tags; len; start;
	if (speaker == nullptr)
	{
		conprint(2, "MSBT Emote called but speaker is set.");
		return;
	}
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
	wobble.Use();
	wobble.SetInt("image", 0);
	wobble.SetInt("gradient1", 1);
	wobble.SetInt("gradient2", 2);

	auto extensions = VFS::ReadJSON("msbt/content.json");
	if (extensions)
	{
		for (auto extension : extensions->AsObject())
		{
			if (!extension.second->IsString())
			{
				conprint(2, "MSBT extension {} is not a string.", extension.first);
				continue;
			}
			auto val = extension.second->AsString();
			if (val.length() < 4 || val.substr(val.length() - 4) != ".lua")
			{
				//This is a raw string. Convert it to a Lua thing.
				//TODO: escape.
				val = fmt::format("return \"{}\"\r\n", val);
			}
			else
			{
				val = VFS::ReadString(fmt::format("msbt/{}", val));
			}
			msbtPhase1X[extension.first] = val;
		}
	}

	//Text(u8"Truth is... <color:1>the game</color> was rigged\nfrom the start.", 0, "Isabelle", glm::vec4(1, 0.98f, 0.56f, 1), glm::vec4(0.96f, 0.67f, 0.05f, 1));
	//Text(u8"Truth is... <color:1>the game</color> was rigged from the start.",
	//Text("Are you <color:3><str:player></color>? <delay:1000>Hiii! Welcome to <color:2>Project Special K</color>!", 0);
	//Text(TextGet("dlg:sza:wack"), Database::Find<Villager>("psk:cat00", villagers));
	//Text("MSBT JSON/Lua test:\n<test1>\n<test2>", 3);
	//Text(u8"This is ordinary dialogue with a button image in it: \uE0E2 look at that.", Database::Find<Villager>("psk:cat00", villagers));
	//Text("By the President of the United States of America:\nA Proclamation.<break>Whereas, on the twenty-second day of September, in the year of our Lord one thousand eight hundred and sixty-two, a proclamation was issued by the President of the United States, containing, among other things, the following, to wit:", 0);
	//Text("Timmy Turner, my name is DougsDaleDimmaDaleDimmaDimmsDomeDoDiDomeDimmsDimmaDimmaDome, owner of the DougDimmsDimmaDaleDimmaDimmsDomeDoDiDimmaDimmsDaleDimmaDimmsDaleDimmaDome.", 0);
	//Text(u8"<font:0>◗♗♔ⓢ◗ꍏ↳€ ◗♗♔♔ꍏ◗⊙♔€", 0);
	//Text(u8"シリーズに含まれる作品の範囲については、制作時期・代理店や原作者の違いなどから、当初は『バトルフィーバーJ』（1979年 - 1980年）を起点としてカウントされていたが。", 0);
	//Text(u8"Je hebt voor <color:1>Nederlands</color> gekozen. Waarom zijn bijna alle namen hetzelfde gebleven?", 0);

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
			auto start = (int)msbtStart - 1;
			auto len = (int)(msbtEnd - msbtStart) + 2;
			if (func != msbtPhase1.end())
			{
				std::invoke(func->second, this, msbt, start, len);
				i = msbtStart; //-1 because we may have subbed in a new tag.
			}
			else
			{
				//Is it an extension?
				auto func2 = msbtPhase1X.find(msbt[0]);
				if (func2 != msbtPhase1X.end())
				{
					//TODO: allow passing arguments
					Sol.set("msbt", msbt);
					toDisplay.replace(start, len, Sol.script(func2->second).get<std::string>());
					i = msbtStart;
				}
			}
		}
	}
}

void DialogueBox::Wrap()
{
	//TODO: needs wrangling for Japanese.
	size_t lastSpace = 0;
	size_t start = 0;
	auto threeLines = sprender.MeasureText(font, "Mg\nMg\nMg\n", 100).y + 0;
	int lineCount = 0;
	for (size_t i = 0; i < toDisplay.length();)
	{
		if (toDisplay.length() > 7 && toDisplay.substr(i, 7) == "<break>")
		{
			i += 7;
			start = i;
		}

		unsigned int ch;
		size_t size;
		std::tie(ch, size) = GetChar(toDisplay, i);

		auto substr = toDisplay.substr(start, i - start + size);
		auto space = sprender.MeasureText(font, substr, 100);

		if (space.x > 650)
		{
			//scan back for a whitespace.
			bool wasLower = std::islower(ch);
			bool wantHyphen = false;
			bool insert = false;
			size_t newSpacePos = 0xFFFF;

			for (size_t b = i;; b--)
			{
				unsigned int bch;
				size_t bsize;
				std::tie(bch, bsize) = GetChar(toDisplay, b);
				if (std::isblank(bch))
				{
					//found whitespace, replace it.
					newSpacePos = b;
					insert = false;
					wantHyphen = false;
					break;
				}
				else if (b == start || bch == '\n') // || (b > 7 && toDisplay.substr(b - 7, 7) == "<break>"))
				{
					//couldn't find anything on this line, insert a space right here instead.
					newSpacePos = i;
					insert = true;
					break;
				}
				else if (wasLower && std::isupper(bch) && newSpacePos == 0xFFFF)
				{
					//went from lowercase to uppercase.
					newSpacePos = i;
					insert = true;
					wantHyphen = true;
					//keep looking for better options!
					//break;
				}
			}
			if (newSpacePos != 0xFFFF)
			{
				if (insert)
				{
					if (wantHyphen)
						toDisplay.insert(toDisplay.begin() + newSpacePos - 1, '-');
					toDisplay.insert(toDisplay.begin() + newSpacePos, '\n');
				}
				else
					toDisplay[newSpacePos] = '\n';
				lastSpace = newSpacePos;
				newSpacePos = 0xFFFF;
				wasLower = false;
				wantHyphen = false;
				lineCount++;
			}
		}
		i += size;

		//TODO: This bit needs to be looked into.
		//We want at least three lines so LOUD SINGLE LINES don't count.
		if (space.y >= threeLines && lineCount >= 3)
		{
			if (toDisplay[lastSpace] == '\n')
			{
				toDisplay[lastSpace] = '>';
				toDisplay.insert(lastSpace, "<break");
			}
			else
				toDisplay.insert(lastSpace, "<break>");
			i += 7;
			lineCount = 0;
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

	if (state == State::Done)
		state = State::Opening;
	else
		state = State::Writing;
}

void DialogueBox::Text(const std::string& text, int style, const std::string& who, const glm::vec4& tagBack, const glm::vec4& tagInk)
{
	Style(style);
	speaker = nullptr;
	if (style != 3)
	{
		name = who;
		nametagWidth = sprender.MeasureText(1, name, 120).x - 32;
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

void DialogueBox::Text(const std::string& text, VillagerP who)
{
	Style(0);
	speaker = who;
	if (who != nullptr)
	{
		name = who->Name();
		nametagWidth = sprender.MeasureText(1, name, 120).x - 32;
		nametagColor[0] = who->NameTag[0];
		nametagColor[1] = who->NameTag[1];
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
	wobble.SetFloat("time", time);

	sprender.DrawSprite(&wobble, bubble[bubbleNum], glm::vec2(dlgLeft, dlgTop), glm::vec2(dlgWidth * 2, dlgHeight), glm::vec4(0, 0, bubble[bubbleNum].width * 2, bubble[bubbleNum].height), 0, bubbleColor);

	sprender.DrawText(font, displayed, glm::vec2(dlgLeft + (200 * dlgScale), dlgTop + (100 * dlgScale)), textColor, 150 * dlgScale);

	if (!name.empty())
	{
		const auto tagAngle = -2.0f;
		const auto tagPos = glm::vec2((int)(width / 2) - bubble[0].width + (150 * scale), dlgTop + (sinf(time * 2) * 10) * scale);
		const auto tagSize = glm::vec2(nametag[0].z, nametag[0].w) * scale;
		const auto alpha = glm::clamp((tween * 2.0f) - 0.75f, 0.0f, 1.0f);
		nametagColor[0].a = alpha;
		nametagColor[1].a = alpha;

		const auto tagPosL = tagPos;
		const auto tagPosM = tagPosL + glm::rotate(glm::vec2(tagSize.x, 0), glm::radians(tagAngle));
		const auto tagPosR = tagPosM + glm::rotate(glm::vec2(nametagWidth, 0), glm::radians(tagAngle));
		const auto tagPosT = tagPosL + glm::rotate(glm::vec2(tagSize.x - 16, 20), glm::radians(tagAngle));

		sprender.DrawSprite(nametag, tagPosL, tagSize, nametag[0], tagAngle, nametagColor[0], TopLeft);
		sprender.DrawSprite(nametag, tagPosM, glm::vec2(nametagWidth, tagSize.y), nametag[2], tagAngle, nametagColor[0], TopLeft);
		sprender.DrawSprite(nametag, tagPosR, tagSize, nametag[1], tagAngle, nametagColor[0], TopLeft);
		sprender.DrawText(1, name, tagPosT, nametagColor[1], 120 * scale, tagAngle);
	}

	if (state == State::WaitingForKey)
	{
		auto arr = (*UI::controls)[6];
		sprender.DrawSprite(*UI::controls, glm::vec2((width / 2) - (arr.z / 2), height - arr.w - 20), glm::vec2(arr.z, arr.w), arr, 0.0f, UI::themeColors["primary"]);
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

			if (speaker)
			{
				if (std::isalnum(ch) || (ch >= 0x2E80 && ch < 0xF000))
					speaker->SetMouth(rand() % 3);
				else if (!std::isblank(ch))
					speaker->SetMouth(0);
			}

			if (bubbleNum == 3 || Sound == Sound::Bebebese)
				generalSounds["ui"]["voiceMonology"]->Play(true);
		}
		if (delay < glm::epsilon<float>())
			delay = 50;
	}

	if (state == State::WaitingForKey)
	{
		if (Inputs.KeyDown(Binds::Accept))
		{
			Inputs.Clear(Binds::Accept);

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
