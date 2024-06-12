#pragma once

#include <vector>


extern std::vector<Item> items;
extern std::vector<Species> species;
extern std::vector<Personality> personalities;
extern std::vector<Hobby> hobbies;
extern std::vector<Villager> villagers;

namespace Database
{
	extern std::map<std::string, std::vector<std::string>> FilterCategories;
	extern std::map<std::string, bool> Filters;

	extern Texture* ItemIcons;
	extern std::map<std::string, glm::vec4> ItemIconAtlas;

	extern void LoadGlobalStuff();
	
	//Find a database entry by reference-ID.
	template<typename T>
	T* Find(const std::string& target, std::vector<T>* source)
	{
#if 1
		for (int i = 0; i < source->size(); i++)
		{
			T* v = &source->at(i);
			if (v->ID == target)
			{
				return v;
			}
		}
		return nullptr;
#else
		//Arguably better code but...
		auto it = std::find_if(source->begin(), source->end(), [&](T* v) { return v->ID == target; });
		if (it == source->end()) return nullptr;
		return ... what exactly?
#endif
	}

	//Find a database entry by CRC32 hash.
	template<typename T>
	T* Find(unsigned int hash, std::vector<T>* source)
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

	//Find a database entry from a JSONValue. If it's an array of strings, tries each.
	template<typename T>
	T* Find(const JSONValue* value, std::vector<T>* source)
	{
		if (value->IsString())
			return Find<T>(value->AsString(), source);
		if (value->IsArray())
		{
			for (const auto& i : value->AsArray())
			{
				auto t = Find<T>(i, source);
				if (t != nullptr)
					return t;
			}
		}
		return nullptr;
	}
}
