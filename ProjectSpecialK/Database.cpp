#include <filesystem>
#include "SpecialK.h"

#include <stb_image.h>
#include <stb_image_write.h>
#include "engine/Console.h"
#include "engine/Text.h"
#include "engine/TextUtils.h"

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
	std::shared_ptr<Texture> ReactionIcons = nullptr;
	//Required to call item icons by name instead of number, like the atlas provided by Texture.
	std::map<std::string, glm::vec4> ItemIconAtlas;
	std::map<std::string, glm::vec4> ReactionIconAtlas;

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

	static void loadIconsWorker(float* progress, const std::string& path, std::shared_ptr<Texture>* texture, std::map<std::string, glm::vec4>& atlas)
	{
		debprint(0, "Icons: loading {}...", path);

		constexpr int iconSize = 128;
		//constexpr int cols = 16;
		//constexpr int rows = 16;
		//constexpr int sheetW = cols * iconSize;
		//constexpr int sheetH = rows * iconSize;
		int cols = 16;
		int rows = cols;

		unsigned char* sheet;
		int width, height, channels;

		auto startingTime = std::chrono::high_resolution_clock::now();
		auto entries = VFS::Enumerate(fmt::format("icons\\{}\\*.png", path));
		auto endingTime = std::chrono::high_resolution_clock::now();
		auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(endingTime - startingTime);
		debprint(0, "IconsWorker: loading {} took {} milliseconds to grab {} items.", path, ms_int.count(), entries.size());

		if (entries.size() >= cols * rows)
		{
			debprint(1, "Icons: Too many icons! Got {} but can only fit {}.", entries.size(), cols * rows);
			//entries.erase(entries.begin() + (cols * rows), entries.end());
			bool expandToRight = true;
			while (entries.size() >= cols * rows)
			{
				if (expandToRight)
					cols *= 2;
				else
					rows *= 2;
				expandToRight = !expandToRight;
			}
		}

		int sheetW = cols * iconSize;
		int sheetH = rows * iconSize;
		sheet = new unsigned char[(sheetW * sheetH) * 4]{0};

		auto progressStep = progressParts / entries.size();

		stbi_set_flip_vertically_on_load(0);
		int iconNum = 0;
		for (const auto& entry : entries)
		{
			size_t vfsSize = 0;
			auto vfsData = VFS::ReadData(entry.path, &vfsSize);
			unsigned char *data = stbi_load_from_memory((unsigned char*)vfsData.get(), (int)vfsSize, &width, &height, &channels, 0);

			//TODO: this is fragile.
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
			filename = filename.substr(filename.rfind('/') + 1);
			filename = filename.substr(0, filename.find('.'));
			atlas[filename] = glm::vec4(l, t, iconSize, iconSize);

			*progress += progressStep;
		}

#if 0 //#ifdef DEBUG
		{
			stbi_flip_vertically_on_write(1);
			stbi_write_png(fmt::format("{}.png", path).c_str(), sheetW, sheetH, 4, sheet, sheetW * 4);
		}
#endif

		//Loading happens in this thread, but making a texture out of it will be delayed.
		*texture = std::make_shared<Texture>(sheet, sheetW, sheetH, 4);
		free(sheet);
		
		Forget(entries);

		debprint(0, "Icons: generated a sheet for {} entries.", entries.size());
	}

	void LoadIcons(float* progress)
	{
		loadIconsWorker(progress, "items", &ItemIcons, ItemIconAtlas);
		loadIconsWorker(progress, "reactions", &ReactionIcons, ReactionIconAtlas);
	}

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
			auto items = std::vector<std::string>();
			Text::Add(key, val["name"]);
			for (const auto& i : val["items"].as_object())
			{
				auto k = i.first;
				auto v = i.second.as_object();
				Text::Add(k, v["name"]);
				items.push_back(k);

				Filters[k] = v["default"].is_boolean() ? v["default"].as_boolean() : true;
				if (settings.find(k) != settings.end())
					Filters[k] = settings[k].as_boolean();
			}
			FilterCategories[key] = items;
		}
		//delete doc;
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
					target.emplace_back(std::make_shared<T2>((jsonObject&)doc.as_object(), entry.path));
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
			//delete doc;
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

		const char *types[] = {
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
			table.push_back(spec->EnName[0]);
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

		LoadIcons(progress);

		LoadText(progress);
		LoadItems(progress);
		LoadSpecies(progress);
		LoadTraits(progress);
		LoadVillagers(progress);
	}
}
