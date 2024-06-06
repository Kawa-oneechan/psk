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
	void LoadItemIcons()
	{
		conprint(0, "ItemIcons: loading...");

		int width, height, channels;

		const int iconSize = 128;
		const int cols = 16;
		const int rows = 16;
		const int sheetW = cols * iconSize;
		const int sheetH = rows * iconSize;

		unsigned char* sheet = (unsigned char*)malloc((sheetW * sheetH) * 4);
		if (sheet == nullptr)
			throw std::runtime_error("ItemIcons: could not allocate sheet.");
		memset(sheet, 0x00, (sheetW * sheetH) * 4);
		
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
		}

#ifdef DEBUG
		{
			stbi_flip_vertically_on_write(1);
			stbi_write_png("itemicons.png", sheetW, sheetH, 4, sheet, sheetW * 4);
			/*
			FILE* tf;
			fopen_s(&tf, "temp.bin", "wb");
			fwrite(sheet, (sheetW * sheetH) * 4, 1, tf);
			fclose(tf);
			*/
		}
#endif

		//TODO: put itemIcons somewhere else and ONLY call this once OpenGL is running.
		//These functions are dynamically loaded and ALL ARE NULL initially.
		//auto itemIcons = Texture(sheet, sheetW, sheetH, 4, true);
		free(sheet);

		ForgetVFS(entries);

		conprint(0, "ItemIcons: generated a sheet for {} entries.\n", entries.size());
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
					conprint(1, u8" {}\n", e.what());
				}
			}
			else
			{
				conprint(1, "{}: error loading {}.\n", whom, entry.path);
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
		conprint(0, "ItemsDatabase: loading...\n");
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
		conprint(0, "ItemsDatabase: ended up with {} entries.\n", items.size());
	}

	void LoadSpecies()
	{
		conprint(0, "SpeciesDatabase: loading...\n");
		loadWorker<Species>(species, "species/*.json", "SpeciesDatabase");
		auto table = std::vector<std::string>{ "ID", "Name" };
		for (const auto& spec: species)
		{
			table.push_back(spec.ID);
			table.push_back(spec.EnName[0]);
		}
		Table(table, 2);
		conprint(0, "SpeciesDatabase: ended up with {} entries.\n", species.size());
	}

	void LoadTraits()
	{
		conprint(0, "TraitsDatabase: loading...\n");
		loadWorker<Personality>(personalities, "personalities/*.json", "TraitsDatabase");
		loadWorker<Hobby>(hobbies, "hobbies/*.json", "TraitsDatabase");
		conprint(0, "TraitsDatabase: ended up with {} personalities and {} hobbies.\n", personalities.size(), hobbies.size());
	}

	void LoadVillagers()
	{
		conprint(0, "VillagerDatabase: loading...\n");
		loadWorker<Villager>(villagers, "villagers/*.json", "VillagerDatabase");
		auto table = std::vector<std::string>{ "ID", "Name","Hash" };
		for (const auto& villager : villagers)
		{
			table.push_back(villager.ID);
			table.push_back(villager.EnName);
			table.emplace_back(fmt::format("{:08X}", villager.Hash));
		}
		Table(table, 3);
		conprint(0, "VillagerDatabase: ended up with {} entries.\n", villagers.size());
	}

	void LoadGlobalStuff()
	{
		conprint(0, "Starting timer.\n");
		auto startingTime = std::chrono::high_resolution_clock::now();
		
		LoadItemIcons();

		LoadItems();
		LoadSpecies();
		LoadTraits();
		LoadVillagers();

		auto endingTime = std::chrono::high_resolution_clock::now();
		auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(endingTime - startingTime);
		conprint(0, "Loading all this took {} milliseconds.\n", ms_int.count());
	}
}
