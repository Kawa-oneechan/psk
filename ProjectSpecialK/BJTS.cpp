#include "engine/Console.h"
#include "engine/InputsMap.h"
#include "engine/Text.h"
#include "engine/Random.h"
#include "Game.h"
#include "DialogueBox.h"
#include "Player.h"
#include "Types.h"
#include "Database.h"

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

static inline int NumberValue(const std::string& value)
{
	try
	{
		return std::stoi(value);
	}
	catch (std::invalid_argument&)
	{
		auto v = Sol.get_or<int>(value, 0xDEADBEEF);
		if (v == 0xDEADBEEF)
		{
			//Not a global. Try to grab a local.
			int i = 1;
			while (true)
			{
				std::tuple<std::string, int> res;
				res = Sol["getlocal"](2, i);
				auto k = std::get<0>(res);
				v = std::get<1>(res);
				i++;
				if (k == value)
					break;
				if (k.empty())
					return 0;
			}
		}
		return v;
	}
}

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
	(void)(tags);
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
			if (Text::Count(fmt::format("{}:0", key)) == 0)
			{
				//guess not :shrug:
				key = data.substr(start + 1, len - 2);
				key.replace(ppos, 1, "normal");
			}
		}
	}

	auto options = Text::Count(fmt::format("{}:", key));
	if (options == 0)
	{
		conprint(2, "Wordstructor: could not find anything for \"{}\".", key);
		data.replace(start, len, "???WS???");
		return;
	}

	int choice = Random::GetInt((int)options);
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

static void bjtsBells(std::string& data, BJTSParams)
{
	if (tags.size() < 2)
	{
		conprint(2, "Missing parameter in BJTS Bells: {}", data.substr(start, len));
		return;
	}
	auto value = NumberValue(tags[1]);

	//TODO: make the currency a setting. Special-case if it's bells
	//so we can say "X bells" or "X klingels" or whatever.
	auto currency = Database::Currencies["usd"];
	auto symbol = std::get<0>(currency);
	auto rate = std::get<1>(currency);

	data.replace(start, len, fmt::format("{}{:.1}", symbol, value * rate));
}

typedef void(*BJTSFunc)(std::string& data, BJTSParams);

//BJTS functions that actually change the string content.
const std::map<std::string, BJTSFunc> bjtsPhase1 = {
	{ "str", &bjtsStr },
	{ "...", &bjtsEllipses },
	{ "ws", &bjtsWordstruct },
	{ "key", &bjtsKeyControl },
	{ "pad", &bjtsGamepad },
	{ "bells", &bjtsBells },
};

//BJTS functions loaded from Lua scripts.
std::map<std::string, std::string> bjtsPhase1X;
