#pragma once

#include "SpecialK.h"

namespace TileType
{
	constexpr auto Grass = 0;
	constexpr auto Weed = 60;
	constexpr auto Flower = 61;
	constexpr auto River = 62;
	constexpr auto Special = 63;
}

struct MapTile
{
	unsigned long Type : 6;
	unsigned long Elevation : 2;
	unsigned long Weathering : 4;
	unsigned long : 4;
	unsigned long FlowerType : 4;
	unsigned long FlowerGrowth : 2;
	unsigned long Watered : 1;
	unsigned long Blocked : 1;
	unsigned long Edges : 4;
	unsigned long Corners : 4;
};

struct ExtraTile
{
	unsigned char Model;
	unsigned char Rotation;
};

class Map : public Tickable
{
public:
	//Size of an Acre in full tiles
	static const int AcreSize = 16t;
	//How tall is a cliff face, roughly?
	static const int ElevationHeight = 15;

	//Width of the town map in tiles
	int Width{ 0 };
	//Height of the town map in tiles
	int Height{ 0 };

	std::unique_ptr<MapTile[]> Terrain{ nullptr };
	std::unique_ptr<ExtraTile[]> TerrainModels{ nullptr };

	std::vector<void*> Objects;

	ModelP Model;

	//If true, this map is represented with a single Model, probably an interior.
	bool UseModel{ false };

	bool UseDrum{ false };

	bool CanOverrideMusic{ false };
	std::string Music;

	void WorkOutModels();

	//Returns the height of the lowest point from the given coordinate.
	float GetHeight(const glm::vec3& pos);
	//Returns the height of the lowest point at the given tile coordinate.
	float GetHeight(int x, int y);

	void Draw(float) {};
	bool Tick(float) { return true; };

#ifdef DEBUG
	void SaveToPNG();
#endif
};

class Town : public Map
{
private:
	unsigned int weatherSeed{ 0 };
	int weatherRain[24] = { 0 };
	int weatherWind[24] = { 0 };
	std::map<std::string, int> flags;

	void drawWorker(float dt);

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
	
	std::vector<VillagerP> Villagers;
	int Wind{ 0 };
	
	bool SquareGrass{ false };

	Town();

	void GenerateNew(void* generator, int width, int height);

	void Load();
	void Save();

	void StartNewDay();
	void UpdateWeather();

	void SetFlag(const std::string& id, int value);
	void SetFlag(const std::string& id, bool value);
	int GetFlag(const std::string& id, int def = 0);
	bool GetFlag(const std::string& id, bool def = false);

	bool Tick(float dt);
	void Draw(float dt);
};

extern std::shared_ptr<Town> town;
