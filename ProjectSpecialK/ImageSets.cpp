#include <filesystem>
#include <stb_image.h>
#include <stb_image_write.h>
#include "engine/Console.h"
#include "Database.h"
#include "Types.h"

namespace fs = std::experimental::filesystem;

namespace Database
{
	std::shared_ptr<Texture> ItemIcons = nullptr;
	std::shared_ptr<Texture> ReactionIcons = nullptr;

	//See Database.cpp, keep this in sync.
	static const float progressParts = 1.0f / 8.0f;

	static void loadIconsWorker(float* progress, const std::string& path, std::shared_ptr<Texture>* texture)
	{
		debprint(0, "Icons: loading {}...", path);

		constexpr int iconSize = 128;
		//constexpr int cols = 16;
		//constexpr int rows = 16;
		//constexpr int sheetW = cols * iconSize;
		//constexpr int sheetH = rows * iconSize;
		int cols = 4;
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

		std::map<std::string, glm::vec4> workAtlas;

		stbi_set_flip_vertically_on_load(0);
		int iconNum = 0;
		int l = 0, t = 0;
		for (const auto& entry : entries)
		{
			size_t vfsSize = 0;
			auto vfsData = VFS::ReadData(entry.path, &vfsSize);
			unsigned char *data = stbi_load_from_memory((unsigned char*)vfsData.get(), (int)vfsSize, &width, &height, &channels, 0);

			const uint32_t* ps = (uint32_t*)data;
			uint32_t* pt = (uint32_t*)sheet + (((rows * iconSize) - t) * (sheetW) + (l));
			for (int y = 0; y < height; y++)
			{
				pt -= sheetW;
				for (int x = 0; x < width; x++)
				{
					*pt = *ps; ps++; pt++;
				}
				pt -= width;
			}
			l += iconSize;
			if (l >= sheetW)
			{
				l = 0;
				t += iconSize;
			}

			iconNum++;
			stbi_image_free(data);

			auto filename = entry.path;
			filename = filename.substr(filename.rfind('/') + 1);
			filename = filename.substr(0, filename.find('.'));
			workAtlas[filename] = glm::vec4(l, t, iconSize, iconSize);

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
		(*texture)->Atlas(workAtlas);
		free(sheet);
		
		Forget(entries);

		debprint(0, "Icons: generated a sheet for {} entries.", entries.size());
	}

	void LoadIcons(float* progress)
	{
		loadIconsWorker(progress, "items", &ItemIcons);
		loadIconsWorker(progress, "reactions", &ReactionIcons);
	}
}
