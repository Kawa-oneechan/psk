#pragma once

#include "SpecialK.h"

#define MAX_VILLAGERS 64
#define MAX_MAPDIST 256
#define MAX_MAPSIZE (MAX_MAPDIST * MAX_MAPDIST)

enum Hemisphere
{
	North, South
};

enum Weather
{
	Fine, Sunny, Cloudy, RainClouds, Rain, HeavyRain
};

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
		};
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
			unsigned char OffsetX, OffsetY; //how far back this extends
			unsigned short Root;
		} Extension;
		struct
		{
			unsigned short : 16;
			union
			{
				unsigned short Watered;
				unsigned short PatternHash;
			};
		};
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

class Town
{
private:
	unsigned int weatherSeed{ 0 };
	int weatherRain[24] = { 0 };
	int weatherWind[24] = { 0 };

public:
	std::string Name{ "Fuck-All Nowhere" };
	Hemisphere Hemisphere{ Hemisphere::North };
	Villager* Villagers[MAX_VILLAGERS]{ 0 };
	Weather Weather{ Fine };
	int Wind{ 0 };
	LiveTerrainTile Tiles[MAX_MAPSIZE]{ 0 };
	std::vector<void*> Objects;

	Town();

	void Load(void* data);
	void* Save();

	void StartNewDay();
	void UpdateWeather();
};

extern Town town;
