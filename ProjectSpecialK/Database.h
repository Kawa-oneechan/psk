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

	extern std::shared_ptr<Texture> ItemIcons;
	extern std::map<std::string, glm::vec4> ItemIconAtlas;

	extern void LoadGlobalStuff(float* progress);
	
	//Find a database entry by reference-ID.
	template<typename T>
	std::shared_ptr<T> Find(const std::string& target, std::vector<std::shared_ptr<T>>& source)
	{
		auto it = std::find_if(source.begin(), source.end(), [&](std::shared_ptr<T> v)
		{
			return v->ID == target;
		});
		if (it == source.end())
			return nullptr;
		return *it;
	}

	//Find a database entry by CRC32 hash.
	template<typename T>
	std::shared_ptr<T> Find(hash hash, std::vector<std::shared_ptr<T>>& source)
	{
		auto it = std::find_if(source.begin(), source.end(), [&](std::shared_ptr<T> v)
		{
			return v->Hash == hash;
		});
		if (it == source.end())
			return nullptr;
		return *it;
	}

	//Find a database entry from a JSONValue. If it's an array of strings, tries each.
	template<typename T>
	std::shared_ptr<T> FindEx(const jsonValue& value, std::vector<std::shared_ptr<T>>& source)
	{
		if (value.is_string())
			return Find<T>(value.as_string(), source);
		if (value.is_array())
		{
			for (const auto& i : value.as_array())
			{
				auto t = Find<T>(i, source);
				if (t != nullptr)
					return t;
			}
		}
		return nullptr;
	}
}
