#include <filesystem>
#include <chrono>
#include "SpecialK.h"

#include "support/stb_image.h"
#include "support/stb_image_write.h"
#include "Texture.h"

namespace fs = std::experimental::filesystem;

std::vector<Item> items;
std::vector<Species> species;
std::vector<Personality> personalities;
std::vector<Hobby> hobbies;
std::vector<Villager> villagers;

namespace Database
{
	void LoadItemIcons()
	{
		printf("ItemIcons: loading...\n");

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
			fmt::print("ItemIcons: Too many icons! Got {} but can only fit {}.", entries.size(), cols * rows);
			entries.erase(entries.begin() + (cols * rows), entries.end());
		}

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

		fmt::print("ItemIcons: generated a sheet for {} entries.\n", entries.size());
	}

	void LoadItems()
	{
		printf("ItemsDatabase: loading...\n");

		auto entries = EnumerateVFS("items\\tools\\*.json");
		items.reserve(entries.size());
		for (const auto& entry : entries)
		{
			auto doc = ReadJSON(entry.path);

			if (doc != nullptr)
			{
				try
				{
					items.emplace_back(Tool((JSONObject&)(doc->AsObject())));
				}
				catch (std::runtime_error& e)
				{
					fmt::print(WARNING u8" {}\n", e.what());
				}
			}
			else
			{
				fmt::print("ItemsDatabase: error loading {}.\n", entry.path);
			}
			delete doc;
		}
		entries = EnumerateVFS("items\\furniture\\*.json");
		items.reserve(items.capacity() + entries.size());
		for (const auto& entry : entries)
		{
			auto doc = ReadJSON(entry.path);

			if (doc != nullptr)
			{
				try
				{
					items.emplace_back(Furniture((JSONObject&)(doc->AsObject())));
				}
				catch (std::runtime_error& e)
				{
					fmt::print(WARNING u8" {}\n", e.what());
				}
			}
			else
			{
				fmt::print("ItemsDatabase: error loading {}.\n", entry.path);
			}
			delete doc;
		}
		entries = EnumerateVFS("items\\outfits\\*.json");
		items.reserve(items.capacity() + entries.size());
		for (const auto& entry : entries)
		{
			const auto baseString = fs::path(entry.path).filename().stem().string();
			const auto baseName = baseString.c_str();

			auto doc = ReadJSON(entry.path);

			if (doc != nullptr)
			{
				try
				{
					items.emplace_back(Outfit((JSONObject&)(doc->AsObject())));
				}
				catch (std::runtime_error& e)
				{
					fmt::print(WARNING u8" {}\n", e.what());
				}
			}
			else
			{
				fmt::print("ItemsDatabase: error loading {}.\n", baseName);
			}
			delete doc;
		}
		fmt::print("ItemsDatabase: ended up with {} entries.\n", items.size());
	}

	void LoadSpecies()
	{
		printf("SpeciesDatabase: loading...\n");

		auto entries = EnumerateVFS("species\\*.json");
		species.reserve(entries.size());
		for (const auto& entry : entries)
		{
			auto doc = ReadJSON(entry.path);

			if (doc != nullptr)
			{
				try
				{
					species.emplace_back(Species((JSONObject&)doc->AsObject()));
				}
				catch (std::runtime_error& e)
				{
					fmt::print(WARNING u8" {}\n", e.what());
				}
			}
			else
			{
				fmt::print("SpeciesDatabase: error loading {}.\n", entry.path);
				/*
				auto error = doc->GetParseError();
				printf("SpeciesDatabase: %s has an error at offset %u: %s\n",
					baseName,
					(unsigned)doc->GetErrorOffset(),
					json::GetParseError_En(error));
				*/
			}
			delete doc;
	}
		fmt::print("SpeciesDatabase: ended up with {} entries.\n", species.size());
	}

	void LoadTraits()
	{
		printf("TraitsDatabase: loading...\n");

		auto entries = EnumerateVFS("personalities\\*.json");
		personalities.reserve(entries.size());
		for (const auto& entry : entries)
		{
			auto doc = ReadJSON(entry.path);

			if (doc != nullptr)
			{
				try
				{
					personalities.emplace_back(Personality((JSONObject&)(doc->AsObject())));
				}
				catch (std::runtime_error& e)
				{
					fmt::print(WARNING u8" {}\n", e.what());
				}
			}
			else
			{
				fmt::print("TraitsDatabase: error loading {}.\n", entry.path);
			}
			delete doc;
		}
		entries = EnumerateVFS("hobbies\\*.json");
		hobbies.reserve(entries.size());
		for (const auto& entry : entries)
		{
			auto doc = ReadJSON(entry.path);

			if (doc != nullptr)
			{
				try
				{
					hobbies.emplace_back(Hobby((JSONObject&)(doc->AsObject())));
				}
				catch (std::runtime_error& e)
				{
					fmt::print(WARNING u8" {}\n", e.what());
				}
			}
			else
			{
				fmt::print("TraitsDatabase: error loading {}.\n", entry.path);
			}
			delete doc;
		}
		fmt::print("TraitsDatabase: ended up with {} personalities and {} hobbies.\n", personalities.size(), hobbies.size());
	}

	void LoadVillagers()
	{
		printf("VillagerDatabase: loading...\n");

		auto entries = EnumerateVFS("villagers\\*.json");
		villagers.reserve(entries.size());
		for(const auto& entry : entries)
		{
			auto doc = ReadJSON(entry.path);

			if (doc != nullptr)
			{
				try
				{
					auto villager = Villager((JSONObject&)doc->AsObject());
					villagers.push_back(std::move(villager));
				}
				catch (std::runtime_error& e)
				{
					fmt::print(WARNING u8" {}\n", e.what());
				}
			}
			else
			{
				fmt::print("VillagerDatabase: error loading {}.\n", entry.path);
				/*
				auto error = doc->GetParseError();
				printf("VillagerDatabase: %s has an error at offset %u: %s\n",
					baseName,
					(unsigned)doc->GetErrorOffset(),
					json::GetParseError_En(error));
				*/
			}
		}
		fmt::print("VillagerDatabase: ended up with {} entries.\n", villagers.size());
	}

	void LoadGlobalStuff()
	{
		printf("Starting timer.\n");
		auto startingTime = std::chrono::high_resolution_clock::now();
		
		LoadItemIcons();

		LoadItems();
		LoadSpecies();
		LoadTraits();
		LoadVillagers();

		auto endingTime = std::chrono::high_resolution_clock::now();
		auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(endingTime - startingTime);
		fmt::print("Loading all this took {} milliseconds.\n", ms_int.count());
	}
}
