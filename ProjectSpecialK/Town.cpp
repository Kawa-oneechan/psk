#include <random>
#include "engine/Model.h"
#include "engine/JSONUtils.h"
#include "engine/Random.h"
#include "engine/Framebuffer.h"
#include "engine/Console.h"
#include "Database.h"
#include "Game.h"
#include "Town.h"
#include "Background.h"
#include "DateTimePanel.h"
#include "ItemHotbar.h"
#include "Types.h"
#include "Player.h"

extern TextureArray* groundTextureAlbs;
extern TextureArray* groundTextureNrms;
extern TextureArray* groundTextureMixs;
extern TextureArray* grassColors;

Town::Town() : grassColorMap ("grasscolors.png"), grassTexture("design0_mix.png")
{
	Music = "clock";
	CanOverrideMusic = true;
	AllowRedeco = true;
	AllowTools = true;

	if (Random::GetFloat() > 0.75f)
		grassTexture = "squares_mix.png";
	grassCanSnow = true;
	weatherSeed = Random::GetInt();

	Width = AcreSize * 1;
	Height = AcreSize * 1;
	Terrain = std::make_unique<MapTile[]>(Width * Height);
	TerrainModels = std::make_unique<ExtraTile[]>(Width * Height);
	Acres.clear();
	Acres.resize((Width / AcreSize) * (Height / AcreSize));

	/*
	//TEST
	for (int i = 0; i < Width; i++)
	{
		//Top
		Terrain[i].Elevation = 1;
	}
	for (int i = 0; i < Height; i++)
	{
		//Left
		Terrain[(i * Width)].Elevation = 1;
		//Right
		Terrain[(i * Width) + (Width - 1)].Elevation = 1;
	}

	Terrain[(4 * Width) + 2].Type = 1; //single sand tile
	Terrain[(0 * Width) + 2].Type = 2; //single stone tile on cliff

	Terrain[(8 * Width) + 5].Elevation = 2; //single column
	Terrain[(7 * Width) + 4].Elevation = 1;
	Terrain[(7 * Width) + 5].Elevation = 1;
	Terrain[(7 * Width) + 6].Elevation = 1;
	Terrain[(8 * Width) + 4].Elevation = 1;
	Terrain[(8 * Width) + 6].Elevation = 1;
	Terrain[(9 * Width) + 4].Elevation = 1;
	Terrain[(9 * Width) + 5].Elevation = 1;
	Terrain[(9 * Width) + 6].Elevation = 1;

	WorkOutModels();

	//end test
	*/

	UseDrum = true;

	//GenerateNew("mappers/test.lua", 6, 6);
	Load();

#ifdef DEBUG
	SaveToPNG();
#endif
}

void Town::GenerateNew(const std::string& mapper, int width, int height)
{
	Width = AcreSize * width;
	Height = AcreSize * height;

	Terrain = std::make_unique<MapTile[]>(Width * Height);
	TerrainModels = std::make_unique<ExtraTile[]>(Width * Height);
	Acres.clear();
	Acres.resize(width * height);

	auto setTile = [&](int x, int y, int type, int elevation)
	{
		if (x < 0 || y < 0 || x >= Width || y >= Height)
			return false;
		if (type < -1 || type >= 64)
			return false;
		if (elevation < -1 || elevation >= 4)
			return false;
		if (type != -1)
			Terrain[(y * Width) + x].Type = type;
		if (elevation != -1)
			Terrain[(y * Width) + x].Elevation = elevation;
		return true;
	};

	auto raise = [&](int x, int y)
	{
		if (x < 0 || y < 0 || x >= Width || y >= Height)
			return false;
		if (Terrain[(y * Width) + x].Elevation == 3)
			return false;
		Terrain[(y * Width) + x].Elevation++;
		return true;
	};

	auto elevation = [&](int x, int y)
	{
		if (x < 0 || y < 0 || x >= Width || y >= Height)
			return -1;
		return (int)Terrain[(y * Width) + x].Elevation;
	};

	Sol["map"] = sol::new_table();
	Sol["map"]["Width"].set(Width - 1);
	Sol["map"]["Height"].set(Height - 1);
	Sol["map"]["SetTile"].set_function(setTile);
	Sol["map"]["Raise"].set_function(raise);
	Sol["map"]["Elevation"].set_function(elevation);

	Sol.script(VFS::ReadString(mapper));

	Sol["map"] = nullptr;

#ifdef DEBUG
	SaveToPNG();
#endif
}

void Town::Load()
{
	try
	{
		auto json = VFS::ReadSaveJSON("map/town.json");
		auto jsonObj = json.as_object();

		Name = jsonObj["name"].as_string();
		weatherSeed = jsonObj["weather"].as_integer();
		Hemisphere = jsonObj["north"].as_boolean() ? Hemisphere::North : Hemisphere::South;
		grassCanSnow = jsonObj["grassCanSnow"].as_boolean();
		grassColorMap = jsonObj["grassColors"].as_string();
		grassTexture = jsonObj["grassTexture"].as_string();

		Width = jsonObj["width"].as_integer();
		Height = jsonObj["height"].as_integer();
		Terrain = std::make_unique<MapTile[]>(Width * Height);
		TerrainModels = std::make_unique<ExtraTile[]>(Width * Height);
		
		Acres.clear();
		Acres.resize((Width / AcreSize) * (Height / AcreSize));
		auto acres = jsonObj["acres"].as_array();
		for (int i = 0; i < acres.size() && i < Acres.size(); i++)
		{
			if (acres[i].is_null())
				Acres[i].Model.reset();
			else
			{
				Acres[i].Model = std::make_shared<::Model>(fmt::format("field/acres/{}.fbx", acres[i].as_string()));
				Acres[i].ModelName = acres[i].as_string();
			}
		}

		VFS::ReadSaveData(Terrain.get(), "map/map.bin");

		Villagers.clear();
		for (const auto& f : jsonObj["villagers"].as_array())
		{
			Villagers.push_back(Database::Find<Villager>(f.as_string(), villagers));
		}

		LoadObjects(json);

		flags.clear();
		for (const auto& f : jsonObj["flags"].as_object())
		{
			flags[f.first] = f.second.as_integer();
		}
	}
	catch (std::runtime_error&)
	{
		//Nothing to load.
		weatherSeed = 0; //Use this as a sign that there was no town data.
	}

	WorkOutModels();

	People.push_back(&thePlayer);
	for (auto& v : Villagers)
		People.push_back(v.get());
}

void Town::Save()
{
	conprint(0, "Saving...");
	auto json = json5pp::object({});

	VFS::WriteSaveData("map/map.bin", (void*)Terrain.get(), sizeof(MapTile) * Width * Height);

	json.as_object()["width"] = Width;
	json.as_object()["height"] = Height;
	json.as_object()["weather"] = (int)weatherSeed;
	json.as_object()["name"] = Name;
	json.as_object()["north"] = Hemisphere == Hemisphere::North;

	json.as_object()["grassTexture"] = grassTexture;
	json.as_object()["grassCanSnow"] = grassCanSnow;
	json.as_object()["grassColors"] = grassColorMap;

	auto villagersArray = json5pp::array({});
	for (const auto& i : Villagers)
	{
		villagersArray.as_array().push_back(i->ID);
	}
	json.as_object()["villagers"] = std::move(villagersArray);

	auto acresArray = json5pp::array({});
	for (const auto& i : Acres)
	{
		if (i.Model)
			acresArray.as_array().push_back(i.ModelName);
		else
			acresArray.as_array().push_back(nullptr);
	}
	json.as_object()["acres"] = std::move(acresArray);

	SaveObjects(json);

	auto flagsObj = json5pp::object({});
	for (const auto& i : flags)
	{
		flagsObj.as_object()[i.first] = i.second;
	}
	json.as_object()["flags"] = std::move(flagsObj);
	
	VFS::WriteSaveJSON("map/town.json", json);
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
		debprint(0, "Today is {} {}. Let's see.", day, month);

		std::srand(weatherSeed + (month << 8) + (day << 16));

		auto doc = VFS::ReadJSON("weather.json");
		auto json = doc.as_object();
		auto calendar = json[Hemisphere == Hemisphere::North ? "north" : "south"].as_array();

		auto here = calendar[0].as_object();
		const int calNow = (month * 31) + day;
		//NOTE: We assume all entries are sorted by date.
		for (int i = 1; 0 < calendar.size(); i++)
		{
			const auto until = GetJSONVec2(calendar[i].as_object().at("until"));
			const int calHere = ((int)until[1] * 31) + (int)until[0];
			if (calHere > calNow)
			{
				here = calendar[i].as_object();
				break;
			}
		}

		//Now to pick a weather pattern for this day...
		//std::srand(weatherSeed + calNow);
		std::random_device device;
		std::mt19937 engine(device());
		engine.seed(weatherSeed + calNow);

		std::vector<int> rates;
		int rateTotal = 0;
		for (auto r : here["rates"].as_array())
		{
			rateTotal += r.as_integer();
			rates.push_back(r.as_integer());
		}
		std::uniform_int_distribution<> dist(0, rateTotal);
		auto roll = dist(engine);
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

		auto patterns = json["patterns"].as_array();
		auto pattern = patterns[pick].as_object();
		debprint(0, "Weather: picked {}, \"{}\".", pick, pattern["id"].as_string());
		auto rain = pattern["rain"].as_array();
		auto wind = pattern["wind"].as_array();
		for (int i = 0; i < 24; i++)
		{
			weatherRain[i] = rain[i].as_integer();
			weatherWind[i] = wind[i].as_integer();
		}
	}

	//Reset textures so snow can appear/disappear
	delete[] groundTextureAlbs;
	groundTextureAlbs = nullptr;
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
		auto windStrength = ((1 << baseWind) - 1) + Random::GetInt(3);
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

void Town::Draw(float dt)
{
	if (groundTextureAlbs == nullptr)
	{
		std::vector<std::string> groundAlbs, groundNrms, groundMixs;
		for (const auto& d : VFS::Enumerate("field/ground/design*_alb.png"))
			groundAlbs.push_back(d.path);
		for (const auto& d : VFS::Enumerate("field/ground/design*_nrm.png"))
			groundNrms.push_back(d.path);
		for (const auto& d : VFS::Enumerate("field/ground/design*_mix.png"))
			groundMixs.push_back(d.path);
		//also add user designs somehow.

		if (grassCanSnow && commonUniforms.GrassColor <= 0.052f || commonUniforms.GrassColor >= 0.865f)
			groundMixs[0] = "field/ground/snow_mix.png";
		else
			groundMixs[0] = fmt::format("field/ground/{}", grassTexture);

		groundTextureAlbs = new TextureArray(groundAlbs);
		groundTextureNrms = new TextureArray(groundNrms);
		groundTextureMixs = new TextureArray(groundMixs);
		grassColors = new TextureArray(fmt::format("field/ground/{}", grassColorMap), GL_CLAMP_TO_EDGE, GL_NEAREST);
	}

	Map::Draw(dt);

	//if ((int)Clouds >= (int)Town::Weather::RainClouds)
	//	rainLayer->Draw(dt * timeScale);

	Sprite::FlushBatch();
}
