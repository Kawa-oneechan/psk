#include "SpecialK.h"
#include "Town.h"

//TODO: make this a Tickable so it can handle transitions and such.

extern Audio* bgm;

static JSONObject library = JSONObject();
static std::string currentID;
static std::string currentFile;

void PlayMusic(const std::string& id)
{
	if (library.size() == 0)
		library = ReadJSON("music/music.json")->AsObject();

	if (id == currentID)
		return;

	tm gm;
	auto now = time(nullptr);
	localtime_s(&gm, &now);

	auto entry = library.find(id);
	if (entry == library.end())
	{
		if (id != "fallback")
			PlayMusic("fallback");
		return;
	}

	auto second = entry->second;

	if (second->IsArray() && second->AsArray()[0]->IsObject())
	{
		auto now2 = (gm.tm_hour * 24) + gm.tm_min;
		auto prev = 0;
		for (auto& ranges : second->AsArray())
		{
			auto r = ranges->AsObject();
			auto t = GetJSONVec2(r["to"]);
			auto to = (int)((t[0] * 24) + t[1]);
			if (now2 < to && now2 > prev)
			{
				second = r["file"];
			}
			prev = to;
		}
	}

	while (second->IsArray())
	{
		auto arr = second->AsArray();
		second = arr[std::rand() % arr.size()];
	}

	if (!second->IsString())
	{
		conprint(1, "PlayMusic: could not figure out \"{}\", did not end up with a string.", id);
		return;
	}

	auto file = second->AsString();

	{
		auto weather = "sunny";
		if (town.Clouds >= Town::Weather::RainClouds)
			weather = "rainy";
		auto tpos = file.find("{time}");
		if (tpos != std::string::npos)
			file = file.replace(tpos, 6, fmt::format("{:02}", gm.tm_hour));

		{
			auto preWeather = file;
			tpos = file.find("{weather}");
			if (tpos != std::string::npos)
				file = file.replace(tpos, 9, weather);
			
			if (EnumerateVFS(file).size() == 0)
			{
				//File does not exist. Try sunny weather first.
				file = preWeather.replace(tpos, 9, "sunny");
				if (EnumerateVFS(file).size() == 0)
				{
					//That too? Okay, try *no* weather!
					file = preWeather.replace(tpos, 9, "");
				}
			}
		}
	}

	if (currentFile != file)
	{
		delete bgm;
		bgm = new Audio(file);
		bgm->Play();
	}

	currentFile = file;
	currentID = id;
}
