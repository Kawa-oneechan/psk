#pragma once

#include "SpecialK.h"

#define MAX_VILLAGERS 64

enum Hemisphere
{
	North, South
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
	Villager* Villagers[MAX_VILLAGERS];

	void StartNewDay();
	std::tuple<int, int> GetWeather();
};
