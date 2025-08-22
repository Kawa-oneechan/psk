#include "engine/Console.h"
#include "engine/InputsMap.h"
#include "engine/Text.h"
#include "engine/Random.h"
#include "Game.h"
#include "DialogueBox.h"
#include "Player.h"

static const char* bindingNames[] = {
	"up", "down", "left", "right",
	"accept", "back", "pageup", "pagedown",
	"walkn", "walkw", "walks", "walke",
	"interact", "pickup",
	"cameracw", "cameraccw", "cameraup", "cameradown",
	"inventory", "map", "react",
	"hotbar1", "hotbar2", "hotbar3", "hotbar4", "hotbar5",
	"hotbar6", "hotbar7", "hotbar8", "hotbar9", "hotbar10",
	"console"
};

static void bjtsStr(std::string& data, BJTSParams)
{
	if (tags.size() < 2)
	{
		conprint(2, "Missing parameter in BJTS Str: {}", data.substr(start, len));
		return;
	}

	auto speaker = dlgBox->Speaker();

	if (tags[1] == "...")
		data.replace(start, len, Text::Get("str:fix:001"));
	else if (tags[1] == "player")
		data.replace(start, len, thePlayer.Name);
	else if (tags[1] == "kun")
		data.replace(start, len, Text::Get("str:kun"));
	else if (tags[1] == "vname")
	{
		if (!speaker)
		{
			conprint(2, "BJTS Str called for villager name, but no speaker is set.");
			data.replace(start, len, "Buggsy");
			return;
		}
		data.replace(start, len, speaker->Name());
	}
	else if (tags[1] == "vspec")
	{
		if (!speaker)
		{
			conprint(2, "BJTS Str called for villager species, but no speaker is set.");
			data.replace(start, len, "bug");
			return;
		}
		data.replace(start, len, speaker->SpeciesName());
	}
	else if (tags[1] == "catchphrase")
	{
		if (!speaker)
		{
			conprint(2, "BJTS Str called for villager catchphrase, but no speaker is set.");
			data.replace(start, len, "bugbug");
			return;
		}
		data.replace(start, len, speaker->Catchphrase());
	}
}

static void bjtsEllipses(std::string& data, BJTSParams)
{
	tags;
	auto fakeTags = std::vector<std::string>
	{
		"str", "..."
	};
	bjtsStr(data, fakeTags, start, len);
}

static void bjtsWordstruct(std::string& data, BJTSParams)
{
	if (tags.size() < 2)
	{
		conprint(2, "Missing parameter in BJTS Wordstruct: {}", data.substr(start, len));
		return;
	}

	//Grab the entire thing, not just individual tags.
	//This method is cheaper than re-joining the tags.
	auto key = data.substr(start + 1, len - 2);

	auto speaker = dlgBox->Speaker();

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
				key = data.substr(start + 1, len - 2);
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
		data.replace(start, len, "???WS???");
		return;
	}

	int choice = rnd::GetInt(options - 1);
	data.replace(start, len, Text::Get(fmt::format("{}:{}", key, choice)));
}

static void bjtsKeyControl(std::string& data, BJTSParams)
{
	if (tags.size() < 2)
	{
		conprint(2, "Missing parameter in BJTS Key: {}", data.substr(start, len));
		return;
	}
	if (tags[1] == "arrows")
	{
		data.replace(start, len, fmt::format("{}/{}/{}/{}",
			Inputs.Keys[(int)Binds::WalkN].Name,
			Inputs.Keys[(int)Binds::WalkS].Name,
			Inputs.Keys[(int)Binds::WalkE].Name,
			Inputs.Keys[(int)Binds::WalkW].Name
		));
		return;
	}
	else  if (tags[1] == "updown")
	{
		data.replace(start, len, fmt::format("{}/{}",
			Inputs.Keys[(int)Binds::Up].Name,
			Inputs.Keys[(int)Binds::Down].Name
		));
		return;
	}
	else  if (tags[1] == "page")
	{
		data.replace(start, len, fmt::format("{}/{}",
			Inputs.Keys[(int)Binds::PageUp].Name,
			Inputs.Keys[(int)Binds::PageDown].Name
		));
		return;
	}
	for (int i = 0; i < NumKeyBinds; i++)
	{
		if (tags[1] == bindingNames[i])
		{
			data.replace(start, len, Inputs.Keys[i].Name);
			return;
		}
	}
}

static void bjtsGamepad(std::string& data, BJTSParams)
{
	if (tags.size() < 2)
	{
		conprint(2, "Missing parameter in BJTS Pad: {}", data.substr(start, len));
		return;
	}
	if (tags[1] == "dpad")
	{
		data.replace(start, len, fmt::format("{}/{}/{}/{}",
			GamepadPUAMap[Inputs.Keys[(int)Binds::WalkN].GamepadButton],
			GamepadPUAMap[Inputs.Keys[(int)Binds::WalkS].GamepadButton],
			GamepadPUAMap[Inputs.Keys[(int)Binds::WalkE].GamepadButton],
			GamepadPUAMap[Inputs.Keys[(int)Binds::WalkW].GamepadButton]
		));
		return;
	}
	else if (tags[1] == "updown")
	{
		data.replace(start, len, fmt::format("{}/{}",
			GamepadPUAMap[Inputs.Keys[(int)Binds::Up].GamepadButton],
			GamepadPUAMap[Inputs.Keys[(int)Binds::Down].GamepadButton]
		));
		return;
	}
	for (int i = 0; i < NumKeyBinds; i++)
	{
		if (tags[1] == bindingNames[i])
		{
			data.replace(start, len, GamepadPUAMap[Inputs.Keys[i].GamepadButton]);
			return;
		}
	}
}

typedef void(*BJTSFunc)(std::string& data, BJTSParams);

//BJTS functions that actually change the string content.
const std::map<std::string, BJTSFunc> bjtsPhase1 = {
	{ "str", &bjtsStr },
	{ "...", &bjtsEllipses },
	{ "ws", &bjtsWordstruct },
	{ "key", &bjtsKeyControl },
	{ "pad", &bjtsGamepad },
};

//BJTS functions loaded from Lua scripts.
std::map<std::string, std::string> bjtsPhase1X;
