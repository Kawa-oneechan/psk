#include "Town.h"

float Map::GetHeight(const glm::vec3& pos)
{
	/*
	TODO: check if there's a building, special acre, or placed object here.
	If so, use proper ray casting to find out the answer.
	*/
	int tx = (int)pos.x;
	int ty = (int)pos.y;
	auto tile = Terrain[tx + (ty * Width)];
	return (float)tile.Elevation * ElevationHeight;
}

float Map::GetHeight(int x, int y)
{
	return GetHeight(glm::vec3(x, y, 100));
}


Town::Town()
{
	weatherSeed = std::rand();

	map.Width = Map::AcreSize * 1;
	map.Height = Map::AcreSize * 1;

	map.Terrain = std::make_unique<LiveTerrainTile[]>(map.Width * map.Height);
	for (int i = 0; i < map.Width; i++)
	{
		map.Terrain[i].Elevation = 1;
	}
	for (int i = 0; i < map.Height; i++)
	{
		map.Terrain[(i * map.Width)].Elevation = 1;
		map.Terrain[(i * map.Width) + (map.Width - 1)].Elevation = 1;
	}

	map.UseDrum = true;
}

void Town::GenerateNew(void* generator, int width, int height)
{
	generator;

	map.Width = Map::AcreSize * width;
	map.Height = Map::AcreSize * height;

	map.Terrain = std::make_unique<LiveTerrainTile[]>(map.Width * map.Height);
}

void Town::Load()
{
	try
	{
		auto json = VFS::ReadSaveJSON("map/flags.json");
		flags.clear();
		for (const auto& f : json->AsObject())
		{
			flags[f.first] = f.second->AsInteger();
		}

	}
	catch (std::runtime_error&)
	{ //-V565
		//Nothing to load.
		//TODO: DWI
	}
}

void Town::Save()
{
	JSONObject json;
	for (const auto& i : flags)
	{
		json[i.first] = new JSONValue(i.second);
	}
	auto val = JSONValue(json);
	VFS::WriteSaveJSON("map/flags.json", &val);
}

void Town::StartNewDay()
{
	//Select weather
	{
		tm gm;
		auto now = time(nullptr);
		localtime_s(&gm, &now);
		const auto month = gm.tm_mon + 1;
		const auto day = gm.tm_mday;
		conprint(0, "Today is {} {}. Let's see.", day, month);

		std::srand(weatherSeed + (month << 8) + (day << 16));

		auto json = VFS::ReadJSON("weather.json")->AsObject();
		auto calendar = json[Hemisphere == Hemisphere::North ? "north" : "south"]->AsArray();

		auto here = calendar[0]->AsObject();
		const int calNow = (month * 31) + day;
		//NOTE: We assume all entries are sorted by date.
		for (int i = 1; 0 < calendar.size(); i++)
		{
			auto c = calendar[i]->AsObject(); //-V836 TODO: figure this out
			const auto until = GetJSONVec2(c["until"]);
			const int calHere = ((int)until[1] * 31) + (int)until[0];
			if (calHere > calNow)
			{
				here = calendar[i]->AsObject();
				break;
			}
		}

		//Now to pick a weather pattern for this day...
		std::srand(weatherSeed + calNow);

		std::vector<int> rates;
		int rateTotal = 0;
		for (auto r : here["rates"]->AsArray())
		{
			rateTotal += r->AsInteger();
			rates.push_back(r->AsInteger());
		}
		auto roll = std::rand() % rateTotal;
		auto pick = 0;
		for (int i = 0; i < rates.size(); i++)
		{
			if (roll < rates[i])
			{
				pick = i;
				break;
			}
			roll -= rates[i];
		}

		auto patterns = json["patterns"]->AsArray();
		auto pattern = patterns[pick]->AsObject();
		conprint(0, "Weather: picked {}, \"{}\".", pick, pattern["id"]->AsString());
		auto rain = pattern["rain"]->AsArray();
		auto wind = pattern["wind"]->AsArray();
		for (int i = 0; i < 24; i++)
		{
			weatherRain[i] = rain[i]->AsInteger();
			weatherWind[i] = wind[i]->AsInteger();
		}
	}
}

void Town::UpdateWeather()
{
	tm gm;
	auto now = time(nullptr);
	localtime_s(&gm, &now);
	auto hour = gm.tm_hour;

	Clouds = static_cast<Town::Weather>(weatherRain[hour]);
	
	if (weatherWind[hour] == 0)
		Wind = 0;
	else
	{
		auto baseWind = std::abs(weatherWind[hour]) - 1;
		auto landIsEast = ((weatherSeed >> 16) % 2) == 1;
		auto windIsLand = weatherWind[hour] < 0;
		auto windStrength = ((1 << baseWind) - 1) + (std::rand() % 3);
		if (windIsLand && landIsEast)
			windStrength = -windStrength;
		Wind = windStrength;
	}
}

void Town::SetFlag(const std::string& id, int value)
{
	flags[id] = value;
}

void Town::SetFlag(const std::string& id, bool value)
{
	SetFlag(id, (int)value);
}

int Town::GetFlag(const std::string& id, int def)
{
	auto v = flags.find(id);
	if (v == flags.end())
		return def;
	return v->second;
}

bool Town::GetFlag(const std::string& id, bool def)
{
	return GetFlag(id, (int)def) > 0;
}

float Town::GetHeight(const glm::vec3& pos)
{
	return map.GetHeight(pos);
}
float Town::GetHeight(int x, int y)
{
	return map.GetHeight(x, y);
}

Town town;
