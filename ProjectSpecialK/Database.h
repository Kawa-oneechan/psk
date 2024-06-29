#pragma once

#include <vector>


extern std::vector<ItemP> items;
extern std::vector<SpeciesP> species;
extern std::vector<PersonalityP> personalities;
extern std::vector<HobbyP> hobbies;
extern std::vector<VillagerP> villagers;

namespace Database
{
	extern std::map<std::string, std::vector<std::string>> FilterCategories;
	extern std::map<std::string, bool> Filters;

	extern Texture* ItemIcons;
	extern std::map<std::string, glm::vec4> ItemIconAtlas;

	extern void LoadGlobalStuff();
	
	//Find a database entry by reference-ID.
	template<typename T>
	std::shared_ptr<T> Find(const std::string& target, std::vector<std::shared_ptr<T>>& source)
	{
#if 1
		for (int i = 0; i < source.size(); i++)
		{
			auto v = source.at(i);
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
	std::shared_ptr<T> Find(unsigned int hash, std::vector<std::shared_ptr<T>>& source)
	{
		for (int i = 0; i < source.size(); i++)
		{
			auto v = source.at(i);
			if (v->Hash == hash)
			{
				return v;
			}
		}
		return nullptr;
	}

	//Find a database entry from a JSONValue. If it's an array of strings, tries each.
	template<typename T>
	std::shared_ptr<T> Find(const JSONValue* value, std::vector<std::shared_ptr<T>>& source)
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
