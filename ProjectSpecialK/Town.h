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

#pragma warning(push)
#pragma warning(disable: 4201)

//Describes both placed objects and dropped items.
struct MapItem
{
	InventoryItemP Item; //The actual item.
	glm::vec2 Position; //Where on the map the item is placed/dropped.
	int State; //Extra state. Meaning depends on the item.
	union
	{
		struct
		{
			int Rotation : 2; //If it's placed, which orientation is it in.
			int Layer : 4;
			bool Fixed : 1; //If it's placed, is it wrenched in place?
			bool Dropped : 1; //Is this a one-tile dropped item icon?
		};
		int Placement;
	};
};

struct Acre
{
	ModelP Model; //nullptr if this is a regular no-bullshit acre.
	std::string ModelName;
	std::vector<MapItem> Objects;
};

#pragma warning(pop)

class Map : public Tickable
{
private:
	void drawWorker(float dt);

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

	std::vector<Acre> Acres;

	ModelP Model;

	//If true, this map is represented with a single Model, probably an interior.
	bool UseModel{ false };
	/*Note that even if this is true, there are still MapTiles! Map::Draw()
	may not draw any tiles, but other things still need to know about things
	like floor sounds.
	*/

	//If true, player is allowed to drop, place, move, and pick up items.
	bool AllowRedeco{ false };
	//If true, player is allowed to carry tools outside of special animations.
	bool AllowTools{ false };

	bool UseDrum{ false };

	bool CanOverrideMusic{ false };
	std::string Music;

	void WorkOutModels();

	//Returns the height of the lowest point from the given coordinate.
	float GetHeight(const glm::vec3& pos);
	//Returns the height of the lowest point at the given tile coordinate.
	float GetHeight(int x, int y);

	void Draw(float);
	bool Tick(float);

	void SaveObjects(JSONObject& json);
	void LoadObjects(JSONObject& json);

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
