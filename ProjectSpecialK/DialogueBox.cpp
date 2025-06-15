#include "DialogueBox.h"
#include "PanelLayout.h"
#include "InputsMap.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

constexpr float tweenTimeScale{ 5.0f };
constexpr float wobbleTimeScale{ 0.25f };

void DialogueBox::bjtsStr(BJTSParams)
{
	if (tags.size() < 2)
	{
		conprint(2, "Missing parameter in BJTS Str: {}", toDisplay.substr(start, len));
		return;
	}
	if (tags[1] == "...")
		toDisplay.replace(start, len, Text::Get("str:fix:001"));
	else if (tags[1] == "player")
		toDisplay.replace(start, len, thePlayer.Name);
	else if (tags[1] == "kun")
		toDisplay.replace(start, len, Text::Get("str:kun"));
	else if (tags[1] == "vname")
	{
		if (!speaker)
		{
			conprint(2, "BJTS Str called for villager name, but no speaker is set.");
			toDisplay.replace(start, len, "Buggsy");
			return;
		}
		toDisplay.replace(start, len, speaker->Name());
	}
	else if (tags[1] == "vspec")
	{
		if (!speaker)
		{
			conprint(2, "BJTS Str called for villager species, but no speaker is set.");
			toDisplay.replace(start, len, "bug");
			return;
		}
		toDisplay.replace(start, len, speaker->Species());
	}
	else if (tags[1] == "catchphrase")
	{
		if (!speaker)
		{
			conprint(2, "BJTS Str called for villager catchphrase, but no speaker is set.");
			toDisplay.replace(start, len, "bugbug");
			return;
		}
		toDisplay.replace(start, len, speaker->Catchphrase());
	}
}

void DialogueBox::bjtsEllipses(BJTSParams)
{
	tags;
	auto fakeTags = std::vector<std::string>
	{
		"str", "..."
	};
	bjtsStr(fakeTags, start, len);
}

void DialogueBox::bjtsWordstruct(BJTSParams)
{
	if (tags.size() < 2)
	{
		conprint(2, "Missing parameter in BJTS Wordstruct: {}", toDisplay.substr(start, len));
		return;
	}

	//Grab the entire thing, not just individual tags.
	//This method is cheaper than re-joining the tags.
	auto key = toDisplay.substr(start + 1, len - 2);

	auto ppos = key.find('?');
	if (ppos != key.npos)
	{
		//Use speaker's personality, unless there
		//is no speaker in which case we use "normal".
		//But if the speaker's personality isn't an option,
		//reset and use "normal" after all.
		if (!speaker)
			key.replace(ppos, 1, "normal");
		else
		{
			key.replace(ppos, 1, speaker->personality->ID);
			//check if this is available
			auto result = Text::Get(fmt::format("{}:0", key));
			if (result.length() >= 3 && result.substr(0, 3) == "???")
			{
				//guess not :shrug:
				key = toDisplay.substr(start + 1, len - 2);
				key.replace(ppos, 1, "normal");
			}
		}
	}

	//Count the number of options
	int options = 0;
	for (int i = 0; i < 32; i++)
	{
		auto result = Text::Get(fmt::format("{}:{}", key, i));
		if (result.length() >= 3 && result.substr(0, 3) == "???")
		{
			options = i;
			break;
		}
	}
	if (options == 0)
	{
		conprint(2, "Wordstructor: could not find anything for \"{}\".", key);
		toDisplay.replace(start, len, "???WS???");
		return;
	}
	
	int choice = rnd::getInt(options);
	toDisplay.replace(start, len, Text::Get(fmt::format("{}:{}", key, choice)));
}

void DialogueBox::bjtsDelay(BJTSParams)
{
	len; start;
	if (tags.size() < 2)
	{
		conprint(2, "Missing parameter in BJTS Delay: {}", toDisplay.substr(start, len));
		return;
	}
	else if (tags.size() == 2)
	{
		delay = (float)std::stoi(tags[1]);
	}
	else
	{
		if (!speaker)
		{
			conprint(2, "BJTS Delay called with personality dependency, but no speaker is set.");
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

void DialogueBox::bjtsEmote(BJTSParams)
{
	tags; len; start;
	if (!speaker)
	{
		conprint(2, "BJTS Emote called but no speaker is set.");
		return;
	}
	//TODO LATER
}

void DialogueBox::bjtsBreak(BJTSParams)
{
	tags; len; start;
	state = State::WaitingForKey;
}

void DialogueBox::bjtsClear(BJTSParams)
{
	tags; len; start;
	displayed.clear();
}

void DialogueBox::bjtsEnd(BJTSParams)
{
	tags; len; start;
	state = State::Closing;
}
void DialogueBox::bjtsPass(BJTSParams)
{
	tags;
	displayed += toDisplay.substr(start, len);
}

DialogueBox::DialogueBox()
{
	auto extensions = VFS::ReadJSON("bjts/content.json");
	if (extensions)
	{
		for (auto extension : extensions->AsObject())
		{
			if (!extension.second->IsString())
			{
				conprint(2, "BJTS extension {} is not a string.", extension.first);
				continue;
			}
			auto val = extension.second->AsString();
			if (val.length() < 4 || val.substr(val.length() - 4) != ".lua")
			{
				//This is a raw string. Convert it to a Lua thing.
				//And for that, we need to escape quotes!
				ReplaceAll(val, "\"", "\\\"");
				//Possibly other things to but IDCRN.

				val = fmt::format("return \"{}\"\r\n", val);
			}
			else
			{
				val = VFS::ReadString(fmt::format("bjts/{}", val));
			}
			bjtsPhase1X[extension.first] = val;
		}
	}

	//Text(u8"Truth is... <color:1>the game</color> was rigged\nfrom the start.", 0, "Isabelle", glm::vec4(1, 0.98f, 0.56f, 1), glm::vec4(0.96f, 0.67f, 0.05f, 1));
	//Text(u8"Truth is... <color:1>the game</color> was rigged from the start.",
	//Text("Are you <color:3><str:player></color>? <delay:1000>Hiii! Welcome to <color:2>Project Special K</color>!", 0);
	//Text(Get("dlg:sza:wack"), Database::Find<Villager>("psk:cat00", villagers));
	//Text("BJTS JSON/Lua test:\n<test1>\n<test2:bur>", 3);
	//Text(u8"This is ordinary dialogue with a button image in it: \uE0E2 look at that.", Database::Find<Villager>("psk:cat00", villagers));
	//Text("By the President of the United States of America:\nA Proclamation.<break>Whereas, on the twenty-second day of September, in the year of our Lord one thousand eight hundred and sixty-two, a proclamation was issued by the President of the United States, containing, among other things, the following, to wit:", 0);
	//Text("Timmy Turner, my name is DougsDaleDimmaDaleDimmaDimmsDomeDoDiDomeDimmsDimmaDimmaDome, owner of the DougDimmsDimmaDaleDimmaDimmsDomeDoDiDimmaDimmsDaleDimmaDimmsDaleDimmaDome.", 0);
	//Text(u8"<font:0>◗♗♔ⓢ◗ꍏ↳€ ◗♗♔♔ꍏ◗⊙♔€", 0);
	//Text(u8"シリーズに含まれる作品の範囲については、制作時期・代理店や原作者の違いなどから、当初は『バトルフィーバーJ』（1979年 - 1980年）を起点としてカウントされていたが。", 0);
	//Text(u8"Je hebt voor <color:1>Nederlands</color> gekozen. Waarom zijn bijna alle namen hetzelfde gebleven?", 0);
	//Text(u8"<ws:?:greeting>", Database::Find<Villager>("ac:cat01", villagers));
	//Text(u8"If I wore shoes—<size:75>weird<size:100>—I'd wear these\n<color:2>high-tops</color>, and\nstomp around like a shoe guy!", Database::Find<Villager>("ac:cat01", villagers));

	//Text("I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I", 0);
	//Text("IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII", 0);
}

void DialogueBox::Preprocess()
{
	for (size_t i = 0; i < toDisplay.length(); i++)
	{
		auto bjtsStart = toDisplay.find_first_of('<', i);
		if (bjtsStart != std::string::npos)
		{
			bjtsStart++;
			auto bjtsEnd = toDisplay.find_first_of('>', bjtsStart);
			i = bjtsEnd;

			auto bjtsWhole = toDisplay.substr(bjtsStart, bjtsEnd - bjtsStart);
			auto bjts = Split(bjtsWhole, ':');
			auto func = bjtsPhase1.find(bjts[0]);
			auto start = (int)bjtsStart - 1;
			auto len = (int)(bjtsEnd - bjtsStart) + 2;
			if (func != bjtsPhase1.end())
			{
				std::invoke(func->second, this, bjts, start, len);
				i = (size_t)-1; //-1 because we may have subbed in a new tag.
			}
			else
			{
				//Is it an extension?
				auto func2 = bjtsPhase1X.find(bjts[0]);
				if (func2 != bjtsPhase1X.end())
				{
					Sol.set("bjts", bjts);
					toDisplay.replace(start, len, Sol.script(func2->second).get<std::string>());
					i = bjtsStart;
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
	auto threeLines = Sprite::MeasureText(font, "Mg\nMg\nMg\n", 100).y + 0;
	int lineCount = 0;
	for (size_t i = 0; i < toDisplay.length();)
	{
		if (toDisplay.length() > 7 && toDisplay.substr(i, 7) == "<break>")
		{
			i += 7;
			start = i;
		}

		rune ch;
		size_t size;
		std::tie(ch, size) = GetChar(toDisplay, i);

		auto substr = toDisplay.substr(start, i - start + size);
		auto space = Sprite::MeasureText(font, substr, 100);

		if (ch == '\n')
		{
			lastSpace = i;
			lineCount++;
		}

		if (space.x > 650)
		{
			//scan back for a whitespace.
			bool wasLower = (iswlower((wint_t)ch) != 0);
			bool wantHyphen = false;
			bool insert = false;
			size_t newSpacePos = 0xFFFF;

			for (size_t b = i;; b--)
			{
				rune bch;
				size_t bsize;
				std::tie(bch, bsize) = GetChar(toDisplay, b);
				if (iswspace((wint_t)bch))
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
				else if (wasLower && iswupper((wint_t)bch) && newSpacePos == 0xFFFF)
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
		nametagWidth = Sprite::MeasureText(1, name, 120).x - 32;
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
		nametagWidth = Sprite::MeasureText(1, name, 120).x - 32;
		nametagColor[0] = who->NameTag[0];
		nametagColor[1] = who->NameTag[1];
	}
	Text(text);
}

void DialogueBox::Style(int style)
{
	style = glm::clamp(style, 0, 4);

	if (style >= 3) //system
	{
		bubbleColor = UI::themeColors["primary"];
		textColor = UI::textColors[8];
		font = (style == 3) ? 1 : 2;
		bubbleNum = (style == 3) ? 3 : 0;
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

	auto metrics = UI::json["metrics"]->AsObject();

	auto dlgScale = metrics["dialogueScale"]->AsNumber() * scale;

	if (state == State::Opening || state == State::Closing)
		dlgScale *= glm::mix(0.0f, 1.0f, tween);

	auto dlgWidth = bubble[0].width * dlgScale;
	auto dlgHeight = bubble[0].height * dlgScale;
	auto dlgLeft = (width * 0.5) - dlgWidth;
	auto dlgTop = height - dlgHeight - metrics["dialogueGap"]->AsNumber();

	auto wobble = Shaders["wobble"];
	gradient[0].Use(1);
	gradient[1].Use(2);
	wobble->Set("time", time);

	Sprite::DrawSprite(wobble, bubble[bubbleNum], glm::vec2(dlgLeft, dlgTop), glm::vec2(dlgWidth * 2, dlgHeight), glm::vec4(0, 0, bubble[bubbleNum].width * 2, bubble[bubbleNum].height), 0, bubbleColor);

	Sprite::DrawText(font, displayed, glm::vec2(dlgLeft + (metrics["dialogueTextLeft"]->AsNumber() * dlgScale), dlgTop + (metrics["dialogueTextTop"]->AsNumber() * dlgScale)), textColor, 170.0f * dlgScale);

	if (!name.empty())
	{
		const auto tagAngle = metrics["dialogueTagAngle"]->AsNumber();
		const auto tagPos = glm::vec2((int)(width / 2) - bubble[0].width + (metrics["dialogueTagLeft"]->AsNumber() * scale), dlgTop + (sinf(time * 2) * 10) * scale);
		const auto tagSize = glm::vec2(nametag[0].z, nametag[0].w) * scale;
		const auto alpha = glm::clamp((tween * 2.0f) - 0.75f, 0.0f, 1.0f);
		nametagColor[0].a = alpha;
		nametagColor[1].a = alpha;

		const auto tagPosL = tagPos;
		const auto tagPosM = tagPosL + glm::rotate(glm::vec2(tagSize.x, 0), glm::radians(tagAngle));
		const auto tagPosR = tagPosM + glm::rotate(glm::vec2(nametagWidth, 0), glm::radians(tagAngle));
		const auto tagPosT = tagPosL + glm::rotate(glm::vec2(tagSize.x - 16, 20), glm::radians(tagAngle));

		Sprite::DrawSprite(nametag, tagPosL, tagSize, nametag[0], tagAngle, nametagColor[0], Sprite::TopLeft);
		Sprite::DrawSprite(nametag, tagPosM, glm::vec2(nametagWidth, tagSize.y), nametag[2], tagAngle, nametagColor[0], Sprite::TopLeft);
		Sprite::DrawSprite(nametag, tagPosR, tagSize, nametag[1], tagAngle, nametagColor[0], Sprite::TopLeft);
		Sprite::DrawText(1, name, tagPosT, nametagColor[1], 120 * scale, tagAngle);
	}

	if (state == State::WaitingForKey)
	{
		auto arr = (*UI::controls)[6];
		Sprite::DrawSprite(*UI::controls, glm::vec2((width / 2) - (arr.z / 2), height - arr.w - 20), glm::vec2(arr.z, arr.w), arr, 0.0f, UI::themeColors["arrow"]);
	}

	//maybe afterwards port this to the UI Panel system?
}

bool DialogueBox::Tick(float dt)
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

		delay -= dt * 1000.0f;
		if (delay > 0)
			return true;

		if (displayCursor >= toDisplay.length())
		{
			state = State::WaitingForKey;
		}

		delay = 0;

		rune ch;
		size_t size;
		std::tie(ch, size) = GetChar(toDisplay, displayCursor);
		displayCursor += size;

		if (ch == '<')
		{
			auto bjtsEnd = toDisplay.find_first_of('>', displayCursor);
			if (bjtsEnd == -1) goto displayIt;
			auto bjtsStart = displayCursor;
			displayCursor = bjtsEnd + 1;

			auto bjtsWhole = toDisplay.substr(bjtsStart, bjtsEnd - bjtsStart);
			auto bjts = Split(bjtsWhole, ':');
			auto func = bjtsPhase2.find(bjts[0]);
			if (func != bjtsPhase2.end())
			{
				std::invoke(func->second, this, bjts, (int)bjtsStart - 1, (int)(bjtsEnd - bjtsStart) + 2);
			}
			else
				conprint(1, "DialogueBox::Tick: don't know how to handle {}.", bjtsWhole);
		}
		else
		{
		displayIt:
			AppendChar(displayed, ch);

			if (speaker)
			{
				if (std::isalnum(ch) || (ch >= 0x2E80 && ch < 0xF000))
					speaker->SetMouth(rnd::getInt(3));
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
		return false;
	}

	if (state == State::Closing)
	{
		tween -= dt * tweenTimeScale;
		if (tween <= 0.0f)
			state = State::Done;
	}

	return true;
}
