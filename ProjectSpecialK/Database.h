#pragma once

#include <vector>

extern std::vector<Item> items;
extern std::vector<Species> species;
extern std::vector<Personality> personalities;
extern std::vector<Hobby> hobbies;
extern std::vector<Villager> villagers;

namespace Database
{
	extern void LoadGlobalStuff();

	//Find a database entry by reference-ID.
	template<typename T>
	const T* Find(const std::string& target, std::vector<T>* source)
	{
		for (int i = 0; i < source->size(); i++)
		{
			T* v = &source->at(i);
			if (v->ID == target)
			{
				return v;
			}
		}
		return nullptr;
	}

	//Find a database entry by CRC32 hash.
	template<typename T>
	const T* Find(unsigned int hash, std::vector<T>* source)
	{
		for (int i = 0; i < source->size(); i++)
		{
			T* v = &source->at(i);
			if (v->Hash == hash)
			{
				return v;
			}
		}
		return nullptr;
	}
}
