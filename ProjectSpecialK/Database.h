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

	//Find a database entry by reference-ID. The type parameter, if not null, will be set to the kind of reference-ID passed.
	template<typename T>
	const T* Find(const std::string& target, std::vector<T>* source)
	{
		char* t = (char*)target.c_str();

		for (int i = 0; i < source->size(); i++)
		{
			T* v = &source->at(i);
			if (v->ID == t)
			{
				return v;
			}
		}
		return nullptr;
	}
}
