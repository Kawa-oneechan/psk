#include <filesystem>
#include "engine/Console.h"
#include "engine/Text.h"
#include "engine/TextUtils.h"
#include "Database.h"
#include "Types.h"

namespace fs = std::experimental::filesystem;

std::vector<ItemP> items;
std::vector<SpeciesP> species;
std::vector<PersonalityP> personalities;
std::vector<HobbyP> hobbies;
std::vector<VillagerP> villagers;

namespace Database
{
	std::map<std::string, std::vector<std::string>> FilterCategories;
	std::map<std::string, bool> Filters;
	std::map<std::string, std::tuple<std::string, float>> Currencies;

	/*
	1. Item icons
	2. Reaction icons
	3. Text
	4. Items
	5. Species
	6. Personalities
	7. Hobbies
	8. Villagers
	Do not forget to keep that divisor up to date.
	*/
	static const float progressParts = 1.0f / 8.0f;

	extern void LoadIcons(float* progress);

	void LoadContentFilters()
	{
		Filters.clear();
		FilterCategories.clear();
		auto settings = UI::settings["contentFilters"].as_object();
		auto doc = VFS::ReadJSON("filters.json");
		for (const auto& f : doc.as_object())
		{
			auto key = f.first;
			auto val = f.second.as_object();
			auto filterItems = std::vector<std::string>();
			Text::Add(key, val["name"]);
			for (const auto& i : val["items"].as_object())
			{
				auto k = i.first;
				auto v = i.second.as_object();
				Text::Add(k, v["name"]);
				filterItems.push_back(k);

				Filters[k] = v["default"].is_boolean() ? v["default"].as_boolean() : true;
				if (settings.find(k) != settings.end())
					Filters[k] = settings[k].as_boolean();
			}
			FilterCategories[key] = filterItems;
		}
	}

	void LoadCurrencies()
	{
		Currencies.clear();
	
		auto doc = VFS::ReadJSON("currency.json");
		for (const auto& r : doc.as_object())
		{
			auto key = r.first;
			auto data = r.second.as_array();
			auto textKey = fmt::format("currency:{}", key);
			Text::Add(textKey, data[0]);
			Currencies[key] = std::make_tuple(data[1].as_string(), 1.0f / data[2].as_number());
		}
	}

	void LoadText(float* progress)
	{
		auto entries = VFS::Enumerate("text/*.json");
		auto progressStep = progressParts / entries.size();
		auto langID = Text::GetLangCode(gameLang);
		StringToLower(langID);
		for (const auto& entry : entries)
		{
			auto last = entry.path.substr(entry.path.length() - 10);
			if (last[0] == '-')
			{
				if (last.substr(1, 4) != langID)
					continue;
			}
			auto stuff = VFS::ReadJSON(entry.path);
			Text::Add(stuff);
			*progress += progressStep;
		}
	}

	template<typename T1, typename T2>
	void loadWorker(float* progress, std::vector<std::shared_ptr<T1>>& target, const std::string& spec, const std::string& whom)
	{
		auto entries = VFS::Enumerate(spec);
		auto progressStep = progressParts / entries.size();
		target.reserve(entries.size());
		for (const auto& entry : entries)
		{
			auto kek = entry.path.substr(entry.path.length() - 9);
			if (kek == ".mat.json")
				continue;

			auto doc = VFS::ReadJSON(entry.path);

			if (!doc.is_null())
			{
				try
				{
					target.emplace_back(std::make_shared<T2>(doc.as_object(), entry.path));
				}
				catch (std::runtime_error& e)
				{
					conprint(1, u8"{}", e.what());
				}
			}
			else
			{
				conprint(1, "{}: error loading {}.", whom, entry.path);
			}
			*progress += progressStep;
		}
	}

	template<typename T1>
	void loadWorker(float* progress, std::vector<std::shared_ptr<T1>>& target, const std::string& spec, const std::string& whom)
	{
		loadWorker<T1, T1>(progress, target, spec, whom);
	}

	void LoadItems(float* progress)
	{
		debprint(0, "ItemsDatabase: loading...");
		loadWorker<Item>(progress, items, "items/*.json", "ItemsDatabase");

		auto table = std::vector<std::string>{ "ID", "Name", "Type", "Hash" };

		//See Item.h enum class Type
		const std::string types[] = {
			"Thing", "Tool", "Furniture", "Clothing"
		};
		for (const auto& item : items)
		{
			table.push_back(item->ID);
			table.push_back(item->EnName);
			table.emplace_back(types[(int)item->Type]);
			table.emplace_back(fmt::format("{:08X}", item->Hash));
		}
		Table(table, 4);
		debprint(0, "ItemsDatabase: ended up with {} entries.", items.size());
	}

	void LoadSpecies(float* progress)
	{
		debprint(0, "SpeciesDatabase: loading...");
		loadWorker<Species>(progress, species, "species/*.json", "SpeciesDatabase");
		auto table = std::vector<std::string>{ "ID", "Name", "Hash" };
		for (const auto& spec: species)
		{
			table.push_back(spec->ID);
			table.push_back(spec->EnNames[0]);
			table.emplace_back(fmt::format("{:08X}", spec->Hash));
		}
		Table(table, 3);
		debprint(0, "SpeciesDatabase: ended up with {} entries.", species.size());
	}

	void LoadTraits(float* progress)
	{
		debprint(0, "TraitsDatabase: loading...");
		loadWorker<Personality>(progress, personalities, "personalities/*.json", "TraitsDatabase");
		auto table = std::vector<std::string>{ "ID", "Name", "Hash" };
		for (const auto& pers : personalities)
		{
			table.push_back(pers->ID);
			table.push_back(pers->EnName);
			table.emplace_back(fmt::format("{:08X}", pers->Hash));
		}
		Table(table, 3);
		loadWorker<Hobby>(progress, hobbies, "hobbies/*.json", "TraitsDatabase");
		debprint(0, "TraitsDatabase: ended up with {} personalities and {} hobbies.", personalities.size(), hobbies.size());
	}

	void LoadVillagers(float* progress)
	{
		debprint(0, "VillagerDatabase: loading...");
		loadWorker<Villager>(progress, villagers, "villagers/*.json", "VillagerDatabase");

		//Sort villagers by species
		std::sort(villagers.begin(), villagers.end(), [](VillagerP a, VillagerP b)
		{
			if (a->Species()->ID == b->Species()->ID)
				return (a->Name().compare(b->Name()) < 0);
			return (a->Species()->ID.compare(b->Species()->ID) < 0);
		});

		auto table = std::vector<std::string>{ "ID", "Name","Hash", "Flags" };
		for (const auto& villager : villagers)
		{
			table.push_back(villager->ID);
			table.push_back(villager->EnName);
			table.emplace_back(fmt::format("{:08X}", villager->Hash));
			table.emplace_back(fmt::format("{}", villager->IsSpecial() ? "Sp" : "")); //can add more markers
		}
		Table(table, 4);
		debprint(0, "VillagerDatabase: ended up with {} entries.", villagers.size());
	}

	void LoadGlobalStuff(float* progress)
	{
		*progress = 0.0;
		LoadContentFilters();
		LoadCurrencies();

		LoadIcons(progress);

		LoadText(progress);
		LoadItems(progress);
		LoadSpecies(progress);
		LoadTraits(progress);
		LoadVillagers(progress);
	}
}
