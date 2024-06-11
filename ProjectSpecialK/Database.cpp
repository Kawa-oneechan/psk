#include <filesystem>
#include <chrono>
#include "SpecialK.h"

#include "support/stb_image.h"
#include "support/stb_image_write.h"
#include "Console.h"

namespace fs = std::experimental::filesystem;

std::vector<Item> items;
std::vector<Species> species;
std::vector<Personality> personalities;
std::vector<Hobby> hobbies;
std::vector<Villager> villagers;

void Table(std::vector<std::string> data, size_t stride)
{
	size_t width[64] = { 0 };
	auto rows = data.size() / stride;
	for (auto col = 0; col < stride; col++)
	{
		for (auto row = 0; row < rows; row++)
		{
			const auto& cel = data[row * stride + col];
			if (cel.length() > width[col])
				width[col] = cel.length();
		}
	}

	std::string top;
	std::string middle;
	std::string bottom;
	for (auto col = 0; col < stride; col++)
	{
		for (auto i = 0; i < width[col] + 2; i++)
		{
			top += u8"─";
			middle += u8"─";
			bottom += u8"─";
		}
		if (col < stride - 1)
		{
			top += u8"┬";
			middle += u8"┼";
			bottom += u8"┴";
		}
	}
	
	conprint(7, u8"┌{}┐", top);

	for (auto row = 0; row < rows; row++)
	{
		std::string line;
		for (auto col = 0; col < stride; col++)
		{
			const auto& cel = data[row * stride + col];
			line += fmt::format(fmt::format(u8"│ {{:{}}} ", width[col]), cel);
		}
		line += u8"│";
		conprint(7, line);
		if (row == 0)
			conprint(7, fmt::format(u8"├{}┤", middle));
	}

	conprint(7, u8"└{}┘", bottom);
}

namespace Database
{
	std::map<std::string, std::vector<std::string>> FilterCategories;
	std::map<std::string, bool> Filters;

	Texture* ItemIcons = nullptr;
	std::map<std::string, glm::vec4> ItemIconAtlas;

	static unsigned char* sheet;
	static const int iconSize = 128;
	static const int cols = 16;
	static const int rows = 16;
	static const int sheetW = cols * iconSize;
	static const int sheetH = rows * iconSize;

	void LoadItemIcons()
	{
		conprint(0, "ItemIcons: loading...");

		int width, height, channels;

		sheet = new unsigned char[(sheetW * sheetH) * 4];
		//std::memset(sheet, 0x00, (sheetW * sheetH) * 4);
		
		auto entries = EnumerateVFS("itemicons\\*.png");

		if (entries.size() >= cols * rows)
		{
			conprint(1, "ItemIcons: Too many icons! Got {} but can only fit {}.", entries.size(), cols * rows);
			entries.erase(entries.begin() + (cols * rows), entries.end());
		}

		stbi_set_flip_vertically_on_load(0);
		int iconNum = 0;
		for (const auto& entry : entries)
		{
			size_t vfsSize = 0;
			char* vfsData = ReadVFS(entry.path, &vfsSize);
			unsigned char *data = stbi_load_from_memory((unsigned char*)vfsData, (int)vfsSize, &width, &height, &channels, 0);
			free(vfsData);

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
		}

#ifdef DEBUG
		{
			stbi_flip_vertically_on_write(1);
			stbi_write_png("itemicons.png", sheetW, sheetH, 4, sheet, sheetW * 4);
		}
#endif

		//Can't do this here because of multithreading bs.
		//ItemIcons = new Texture(sheet, sheetW, sheetH, 4);
		
		ForgetVFS(entries);

		conprint(0, "ItemIcons: generated a sheet for {} entries.", entries.size());
	}

	//Call this from the main thread.
	void CreateItemIconsTexture()
	{
		if (sheet == nullptr)
		{
			conprint(2, "Tried to create item icons texture with a null sheet. Ran it twice?");
			return;
		}
		ItemIcons = new Texture(sheet, sheetW, sheetH, 4);
		delete[] sheet;
		sheet = nullptr;
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
	void loadWorker(std::vector<T1>& target, const std::string& spec, const std::string& whom)
	{
		auto entries = EnumerateVFS(spec);
		target.reserve(entries.size());
		for (const auto& entry : entries)
		{
			auto doc = ReadJSON(entry.path);

			if (doc != nullptr)
			{
				try
				{
					target.emplace_back(T2((JSONObject&)doc->AsObject()));
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
		}
	}

	template<typename T1>
	void loadWorker(std::vector<T1>& target, const std::string& spec, const std::string& whom)
	{
		loadWorker<T1, T1>(target, spec, whom);
	}

	void LoadItems()
	{
		conprint(0, "ItemsDatabase: loading...");
		loadWorker<Item, Tool>(items, "items/tools/*.json", "ItemsDatabase");
		loadWorker<Item, Furniture>(items, "items/furniture/*.json", "ItemsDatabase");
		loadWorker<Item, Outfit>(items, "items/outfits/*.json", "ItemsDatabase");
		auto table = std::vector<std::string>{ "ID", "Name", "Type", "Hash" };
		for (const auto& item : items)
		{
			table.push_back(item.ID);
			table.push_back(item.EnName);
			table.emplace_back(fmt::format("{:#b}", item.Type));
			table.emplace_back(fmt::format("{:08X}", item.Hash));
		}
		Table(table, 4);
		conprint(0, "ItemsDatabase: ended up with {} entries.", items.size());
	}

	void LoadSpecies()
	{
		conprint(0, "SpeciesDatabase: loading...");
		loadWorker<Species>(species, "species/*.json", "SpeciesDatabase");
		auto table = std::vector<std::string>{ "ID", "Name" };
		for (const auto& spec: species)
		{
			table.push_back(spec.ID);
			table.push_back(spec.EnName[0]);
		}
		Table(table, 2);
		conprint(0, "SpeciesDatabase: ended up with {} entries.", species.size());
	}

	void LoadTraits()
	{
		conprint(0, "TraitsDatabase: loading...");
		loadWorker<Personality>(personalities, "personalities/*.json", "TraitsDatabase");
		loadWorker<Hobby>(hobbies, "hobbies/*.json", "TraitsDatabase");
		conprint(0, "TraitsDatabase: ended up with {} personalities and {} hobbies.", personalities.size(), hobbies.size());
	}

	void LoadVillagers()
	{
		conprint(0, "VillagerDatabase: loading...");
		loadWorker<Villager>(villagers, "villagers/*.json", "VillagerDatabase");
		auto table = std::vector<std::string>{ "ID", "Name","Hash" };
		for (const auto& villager : villagers)
		{
			table.push_back(villager.ID);
			table.push_back(villager.EnName);
			table.emplace_back(fmt::format("{:08X}", villager.Hash));
		}
		Table(table, 3);
		conprint(0, "VillagerDatabase: ended up with {} entries.", villagers.size());
	}

	void LoadGlobalStuff()
	{
		conprint(0, "Starting timer.");
		auto startingTime = std::chrono::high_resolution_clock::now();
		
		LoadContentFilters();

		LoadItemIcons();

		LoadItems();
		LoadSpecies();
		LoadTraits();
		LoadVillagers();

		auto endingTime = std::chrono::high_resolution_clock::now();
		auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(endingTime - startingTime);
		conprint(0, "Loading all this took {} milliseconds.", ms_int.count());
	}
}
