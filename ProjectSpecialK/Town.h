#pragma once

#include "SpecialK.h"

struct SavedMapItem
{
	unsigned int id;
	union
	{
		unsigned char SystemParams;
		struct
		{
			unsigned char Rotation : 2;
			unsigned char Buried : 1;
			unsigned char Dropped : 1;
			unsigned char Status : 3;
			unsigned char Locked : 1;
		} System;
	};
	union
	{
		unsigned char AdditionalParams;
		struct
		{
			unsigned char Type : 2;
			unsigned char ShowItem : 1;
			unsigned char : 1;
			unsigned char Paper : 4;
		} Wrapping;
	};
	unsigned short StackCount;
	unsigned short UseCount;
	struct
	{
		unsigned char Body : 4;
		unsigned char Source : 4;
	} Remake;
	unsigned char FlowerGenes;
	union
	{
		struct
		{
			unsigned char OffsetX, OffsetY; //How far back does this extend?
			unsigned short Root;
		} Extension;
		//... let's be honest, if this format is only for serializing, why bother storing extensions?
		struct
		{
			unsigned short : 16;
			union
			{
				unsigned short Watered;
				unsigned short PatternHash;
			};
		} Flowers;
	};
};

struct SavedTerrainTile
{
	unsigned short Type : 10;
	unsigned short Corners : 4;
	unsigned short Elevation : 2;
};

struct LiveTerrainTile
{
	unsigned char Type;
	unsigned char Variation;
	unsigned char Corners;
	unsigned char Elevation;
};

class Map
{
public:
	//Size of an Acre in full tiles
	static const int AcreSize = 16t;
	//TEMP: how tall is a cliff face, roughly?
	static const int ElevationHeight = 3;

	//Width of the town map in tiles
	int Width{ 0 };
	//Height of the town map in tiles
	int Height{ 0 };

	std::unique_ptr<LiveTerrainTile[]> Terrain{ nullptr };

	std::vector<void*> Objects;

	bool UseDrum{ false };

	//Returns the height of the lowest point from the given coordinate.
	float GetHeight(const glm::vec3& pos);
	//Returns the height of the lowest point at the given tile coordinate.
	float GetHeight(int x, int y);
};

class Town
{
private:
	unsigned int weatherSeed{ 0 };
	int weatherRain[24] = { 0 };
	int weatherWind[24] = { 0 };
	std::map<std::string, int> flags;

	Map map;

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

	//Returns the height of the lowest point from the given coordinate.
	float GetHeight(const glm::vec3& pos);
	//Returns the height of the lowest point at the given tile coordinate.
	float GetHeight(int x, int y);
};

extern Town town;
