#pragma once

#include "Map.h"

class Town : public Map
{
private:

	unsigned int weatherSeed{ 0 };
	int weatherRain[24] = { 0 };
	int weatherWind[24] = { 0 };
	std::map<std::string, int> flags;

	std::string grassTexture;
	std::string grassColorMap;
	bool grassCanSnow;


public:
	std::string Name{ "Fuck-All Nowhere" };

	static const int MaxVillagers{ 64 };

	enum class Hemisphere
	{
		North, South
	} Hemisphere{ Hemisphere::North };

	enum class Weather
	{
		Fine, Sunny, Cloudy, RainClouds, Rain, HeavyRain
	} Clouds{ Weather::Fine };
	
	//Every villager living in this Town.
	std::vector<VillagerP> Villagers;
	int Wind{ 0 };

	Town();

	void GenerateNew(const std::string& mapper, int width, int height);

	void Load();
	void Save();

	void StartNewDay();
	void UpdateWeather();

	void SetFlag(const std::string& id, int value);
	void SetFlag(const std::string& id, bool value);
	int GetFlag(const std::string& id, int def = 0);
	bool GetFlag(const std::string& id, bool def = false);

	void Draw(float dt);
};

extern std::shared_ptr<Town> town;
