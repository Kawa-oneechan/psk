#include "SpecialK.h"
#include "Town.h"

//TODO

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

	auto entry = library.find(id);
	if (entry == library.end())
	{
		if (id != "fallback")
			PlayMusic("fallback");
		return;
	}

	auto second = entry->second;
	while (second->IsArray())
	{
		auto arr = second->AsArray();
		second = arr[std::rand() % arr.size()];
	}

	//TODO: Objects with time ranges.

	if (!second->IsString())
	{
		conprint(1, "PlayMusic: could not figure out \"{}\", did not end up with a string.", id);
		return;
	}

	auto file = second->AsString();

	{
		tm gm;
		auto now = time(nullptr);
		localtime_s(&gm, &now);
		auto weather = "sunny";
		if (town.Clouds >= Town::Weather::RainClouds)
			weather = "rainy";
		auto tpos = file.find("{time}");
		if (tpos != std::string::npos)
			file = file.replace(tpos, 6, fmt::format("{:02}", gm.tm_hour));
		tpos = file.find("{weather}");
		if (tpos != std::string::npos)
			file = file.replace(tpos, 9, weather);
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
