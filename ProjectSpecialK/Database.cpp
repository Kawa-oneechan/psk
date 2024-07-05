#include <filesystem>
#include <chrono>
#include "SpecialK.h"

#include "support/stb_image.h"
#include "support/stb_image_write.h"
#include "Console.h"

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

	std::shared_ptr<Texture> ItemIcons = nullptr;
	//Required to call item icons by name instead of number, like the atlas provided by Texture.
	std::map<std::string, glm::vec4> ItemIconAtlas;

	static const float progressParts = 1.0f / 8.0f;

	void LoadItemIcons(float* progress)
	{
		conprint(0, "ItemIcons: loading...");

		constexpr int iconSize = 128;
		constexpr int cols = 16;
		constexpr int rows = 16;
		constexpr int sheetW = cols * iconSize;
		constexpr int sheetH = rows * iconSize;

		unsigned char* sheet;
		int width, height, channels;

		sheet = new unsigned char[(sheetW * sheetH) * 4];
		
		auto entries = EnumerateVFS("itemicons\\*.png");

		if (entries.size() >= cols * rows)
		{
			conprint(1, "ItemIcons: Too many icons! Got {} but can only fit {}.", entries.size(), cols * rows);
			entries.erase(entries.begin() + (cols * rows), entries.end());
		}

		auto progressStep = progressParts / entries.size();

		stbi_set_flip_vertically_on_load(0);
		int iconNum = 0;
		for (const auto& entry : entries)
		{
			size_t vfsSize = 0;
			auto vfsData = ReadVFS(entry.path, &vfsSize);
			unsigned char *data = stbi_load_from_memory((unsigned char*)vfsData.get(), (int)vfsSize, &width, &height, &channels, 0);

			int l = (iconNum % cols) * iconSize;
			int t = (iconNum / rows) * iconSize;
			uint32_t* ps = (uint32_t*)data;
			uint32_t* pt = (uint32_t*)sheet + (((rows * iconSize)  - t) * (sheetW) + (l));
			for (int y = 0; y < height; y++)
			{
				pt -= sheetW;
				for (int x = 0; x < width; x++)
				{
					*pt = *ps; ps++; pt++;
				}
				pt -= width;
			}

			iconNum++;
			stbi_image_free(data);

			auto filename = entry.path;
			filename = filename.substr(filename.find('/') + 1);
			filename = filename.substr(0, filename.find('.'));
			ItemIconAtlas[filename] = glm::vec4(l, t, iconSize, iconSize);

			*progress += progressStep;
		}

#ifdef DEBUG
		{
			stbi_flip_vertically_on_write(1);
			stbi_write_png("itemicons.png", sheetW, sheetH, 4, sheet, sheetW * 4);
		}
#endif

		//Loading happens in this thread, but making a texture out of it will be delayed.
		ItemIcons = std::make_shared<Texture>(sheet, sheetW, sheetH, 4);
		
		ForgetVFS(entries);

		conprint(0, "ItemIcons: generated a sheet for {} entries.", entries.size());
	}

	void LoadContentFilters()
	{
		Filters.clear();
		FilterCategories.clear();
		auto settings = UI::settings["contentFilters"]->AsObject();
		auto doc = ReadJSON("filters.json");
		for (const auto& f : doc->AsObject())
		{
			auto key = f.first;
			auto val = f.second->AsObject();
			auto items = std::vector<std::string>();
			TextAdd(key, *val["name"]);
			for (const auto& i : val["items"]->AsObject())
			{
				auto k = i.first;
				auto v = i.second->AsObject();
				TextAdd(k, *v["name"]);
				items.push_back(k);

				Filters[k] = v["default"] != nullptr ? v["default"]->AsBool() : true;
				if (settings.find(k) != settings.end())
					Filters[k] = settings[k]->AsBool();
			}
			FilterCategories[key] = items;
		}
	}

	template<typename T1, typename T2>
	void loadWorker(float* progress, std::vector<std::shared_ptr<T1>>& target, const std::string& spec, const std::string& whom)
	{
		auto entries = EnumerateVFS(spec);
		auto progressStep = progressParts / entries.size();
		target.reserve(entries.size());
		for (const auto& entry : entries)
		{
			auto doc = ReadJSON(entry.path);

			if (doc != nullptr)
			{
				try
				{
					target.emplace_back(std::make_shared<T2>((JSONObject&)doc->AsObject(), entry.path));
				}
				catch (std::runtime_error& e)
				{
					conprint(1, u8" {}", e.what());
				}
			}
			else
			{
				conprint(1, "{}: error loading {}.", whom, entry.path);
			}
			delete doc;
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
		conprint(0, "ItemsDatabase: loading...");
		loadWorker<Item, Tool>(progress, items, "items/tools/*.json", "ItemsDatabase");
		loadWorker<Item, Furniture>(progress, items, "items/furniture/*.json", "ItemsDatabase");
		loadWorker<Item, Outfit>(progress, items, "items/outfits/*.json", "ItemsDatabase");
		auto table = std::vector<std::string>{ "ID", "Name", "Type", "Hash" };
		for (const auto& item : items)
		{
			table.push_back(item->ID);
			table.push_back(item->EnName);
			table.emplace_back(fmt::format("{:#b}", item->Type));
			table.emplace_back(fmt::format("{:08X}", item->Hash));
		}
		Table(table, 4);
		conprint(0, "ItemsDatabase: ended up with {} entries.", items.size());
	}

	void LoadSpecies(float* progress)
	{
		conprint(0, "SpeciesDatabase: loading...");
		loadWorker<Species>(progress, species, "species/*.json", "SpeciesDatabase");
		auto table = std::vector<std::string>{ "ID", "Name", "Hash" };
		for (const auto& spec: species)
		{
			table.push_back(spec->ID);
			table.push_back(spec->EnName[0]);
			table.emplace_back(fmt::format("{:08X}", spec->Hash));
		}
		Table(table, 3);
		conprint(0, "SpeciesDatabase: ended up with {} entries.", species.size());
	}

	void LoadTraits(float* progress)
	{
		conprint(0, "TraitsDatabase: loading...");
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
		conprint(0, "TraitsDatabase: ended up with {} personalities and {} hobbies.", personalities.size(), hobbies.size());
	}

	void LoadVillagers(float* progress)
	{
		conprint(0, "VillagerDatabase: loading...");
		loadWorker<Villager>(progress, villagers, "villagers/*.json", "VillagerDatabase");
		auto table = std::vector<std::string>{ "ID", "Name","Hash" };
		for (const auto& villager : villagers)
		{
			table.push_back(villager->ID);
			table.push_back(villager->EnName);
			table.emplace_back(fmt::format("{:08X}", villager->Hash));
		}
		Table(table, 3);
		conprint(0, "VillagerDatabase: ended up with {} entries.", villagers.size());
	}

	void LoadGlobalStuff(float* progress)
	{
		conprint(0, "Starting timer.");
		auto startingTime = std::chrono::high_resolution_clock::now();
		
		*progress = 0.0;
		LoadContentFilters();

		LoadItemIcons(progress);

		LoadItems(progress);
		LoadSpecies(progress);
		LoadTraits(progress);
		LoadVillagers(progress);

		auto endingTime = std::chrono::high_resolution_clock::now();
		auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(endingTime - startingTime);
		conprint(0, "Loading all this took {} milliseconds.", ms_int.count());
	}
}
