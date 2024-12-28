#include "SpecialK.h"
#include "Town.h"

#include "support/stb_image.h"
#include "support/stb_image_write.h"
void TestScaler()
{
	const int scale = 4;

	int width, height, channels;
	size_t vfsSize = 0;
	auto vfsData = VFS::ReadData("test-1.png", &vfsSize);
	unsigned char* src = stbi_load_from_memory((unsigned char*)vfsData.get(), (int)vfsSize, &width, &height, &channels, 0);
	
	auto dst = ScaleImage(src, width, height, channels, 4);

	stbi_flip_vertically_on_write(1);
	stbi_write_png("scale2x.png", width * scale, height * scale, channels, dst, (width * scale) * channels);
	
	delete dst;
	stbi_image_free(src);
}

extern int articlePlease;

void TestVillagerGetting()
{
	auto farrah = Database::Find<Villager>("psk:cat00", villagers);
	//farrah = Database::Find<Villager>(0xEF4A26F4, &villagers);
	if (!farrah)
	{
		conprint(2, "Could not load Farrah. Check the logs.");
		return;
	}

	farrah->defaultClothingID = "acnh:fronttietee/red";

	conprint(0, "Villager getting test: {}, a {}, birthday on {}.", farrah->Name(), StripMSBT(farrah->Species()), farrah->Birthday());
	articlePlease = 0;
	conprint(0, "Default clothing: {}", farrah->defaultClothingID);
	conprint(0, "Portrait: {}", farrah->portraitID);
	conprint(0, "Manifesting...");
	farrah->Manifest();
	conprint(0, "Current clothing: {}", farrah->Clothing->FullName());
	farrah->Depart();

	//Deliberately fail. Should pick the default clothes.
	farrah->PickClothing();
}

void TestInventorySystems()
{
	thePlayer.GiveItem(std::make_shared<InventoryItem>("acnh:snowysweater/black"));
	thePlayer.GiveItem(std::make_shared<InventoryItem>("acnh:pleatherskirt"));
	thePlayer.GiveItem(std::make_shared<InventoryItem>("acnh:flipflops/pink"));
	thePlayer.GiveItem(std::make_shared<InventoryItem>("acnh:fronttietee/red"));
	thePlayer.GiveItem(std::make_shared<InventoryItem>("acnh:denimcutoffs/navyblue"));
	thePlayer.GiveItem(std::make_shared<InventoryItem>("acnh:palatialtankdress"));

	conprint(0, "------Initial------");
	for (auto& i : thePlayer.OnHand)
		conprint(0, "{} ", i != nullptr ? '[' + i->Name() + ']' : "*");

	conprint(0, "------Remove flipflops------");
	thePlayer.RemoveItem(2);
	for (auto& i : thePlayer.OnHand)
		conprint(0, "{} ", i != nullptr ? '[' + i->Name() + ']' : "*");

	conprint(0, "------Switch #3 and #5------");
	thePlayer.SwapItems(3, 5);
	for (auto& i : thePlayer.OnHand)
		conprint(0, "{} ", i != nullptr ? '[' + i->Name() + ']' : "*");

	conprint(0, "------Put #3 in storage-----");
	thePlayer.Store(3);
	for (auto& i : thePlayer.OnHand)
		conprint(0, "{} ", i != nullptr ? '[' + i->Name() + ']' : "*");
	conprint(0, "****");
	for (auto& i : thePlayer.Storage)
		conprint(0, "{} ", i != nullptr ? '[' + i->Name() + ']' : "*");
	conprint(0, "----Take #3 from storage----");
	thePlayer.Retrieve(0);
	for (auto& i : thePlayer.OnHand)
		conprint(0, "{} ", i != nullptr ? '[' + i->Name() + ']' : "*");
	conprint(0, "****");
	for (auto& i : thePlayer.Storage)
		conprint(0, "{} ", i != nullptr ? '[' + i->Name() + ']' : "*");

	conprint(0, "------------");
}


int GetLetterScore(const std::string& text)
{
	//TODO: do this in UTF8, without the std::is____ functions.

	auto trigrams = VFS::ReadString("mailcheck/trigrams.txt");
	int score = 0;

	//Check A: punctuation and capital letters.
	{
		auto last = *(text.end() - 1);
		if (last == '.' || last == '!' || last == '?')
		{
			score += 20;
		}

		auto checkFor = [&score, text](char pct)
		{
			size_t pos = text.find(pct, 0);
			while (pos != std::string::npos)
			{
				if (pos != 0)
					pos++;
				if (pos + 3 >= text.length())
					break;
				bool yea = false;
				for (int i = 0; i < 3; i++)
				{
					if (std::isupper(text[pos + i]))
					{
						score += 10;
						yea = true;
						break;
					}
				}
				if (!yea)
					score -= 10;
				pos = text.find(pct, pos);
				if (pos == 0)
					pos++;
			}
		};

		checkFor('.');
		checkFor('!');
		checkFor('?');
	}

	//Check B: trigrams
	{
		size_t pos = 0;
		int trisFound = 0;
		while (pos != std::string::npos)
		{
			if (pos != 0)
				pos++;
			if (pos + 3 >= text.length())
				break;
			auto tri = text.substr(pos, 3);
			StringToLower(tri);
			auto triPos = trigrams.find(tri);
			if (triPos != std::string::npos)
				trisFound++;
			pos = text.find(' ', pos);
		}
		score += trisFound * 3;
	}

	//Check C: first letter is a capital
	{
		for (auto i = 0; i < text.length(); i++)
		{
			auto c = text[i];
			if (std::isblank(c))
				continue;
			if (std::isupper(c))
				score += 20;
			else if (std::islower(c))
				score -= 10;
			break;
		}
	}

	//Check D: repeated characters
	{
		for (auto i = 0; i < text.length(); i++)
		{
			auto c = text[i];
			auto a = 0;
			for (auto j = i + 1; j < text.length() && j < i + 3; j++, i++)
			{
				if (text[j] == c)
					a++;
				else
					break;
			}
			if (a == 2)
			{
				score -= 50;
				break;
			}
		}
	}

	//Check E: space/non-space ratio
	{
		int spaces = 0;
		int nonspaces = 0;
		for (auto i = 0; i < text.length(); i++)
		{
			auto c = text[i];
			if (std::isblank(c))
				spaces++;
			else
				nonspaces++;
		}
		if (nonspaces == 0 || ((spaces * 100) / nonspaces < 20))
			score -= 20;
		else
			score += 20;
	}

	//Check F: no punctuation within 75 characters
	{
		if (text.length() >= 75)
		{
			auto first = text.substr(0, 75);
			if (first.find('.') == std::string::npos && first.find('!') == std::string::npos && first.find('?') == std::string::npos)
				score -= 150;
		}
	}

	//Check G: at least one space per 32 character cluster
	{
		size_t i = 0;
		while (i < text.length())
		{
			size_t l = 32;
			if (i + l > text.length())
				l = text.length() - i;
			auto chunk = text.substr(i, l);
			if (chunk.find(' ') == std::string::npos)
				score -= 20;
			i += 32;
		}
	}

	return score;
}


void RunTests()
{
	{
		hash item = 0xD25C790C;
		int variant = 1;
		int pattern = 2;
		auto nookCode = NookCode::Encode(item, variant, pattern);

		if (nookCode != "UDQFUUHNIX")
			conprint(2, "NookCode test: expected result \"UDQFUUHNIX\" but got \"{}\".", nookCode);

		item = (hash)-1; variant = -1; pattern = -1;
		NookCode::Decode(nookCode, item, variant, pattern);

		if (item != 0xD25C790C || variant != 1 || pattern != 2)
			conprint(2, "NookCode test: expected results 0xD25C790C 1 2 but got {:08X} {} {}.", item, variant, pattern);
	}

	{
		using T = decltype(town);
		if (std::is_same<T, Town>())
			conprint(0, "town is a Town, yeah.");
		if (std::is_same<T, Map>())
			conprint(0, "town is a Map.");
		if (std::is_same<T, Player>())
			conprint(0, "town is a Player?!");
	}

	std::string toLowerTest = u8"Tendō Akane!";
	StringToLower(toLowerTest);
	conprint(0, "case folding: {}", toLowerTest);
	StringToUpper(toLowerTest);
	conprint(0, "case folding: {}", toLowerTest);

	//test texture re-use
	//auto textureCacheTest = Texture("ui/panels.png");

	TestInventorySystems();
	TestVillagerGetting();

	char ascTime[256] = { 0 };
	auto thanksGiving = GetNthWeekdayOfMonth(10, 4, 4);
	asctime_s(ascTime, thanksGiving);
	auto blackFriday = GetNthWeekdayOfMonth(10, 5, 4);
	asctime_s(ascTime, blackFriday);

	conprint(0, u8"Elevation test: the tile at tile 4×4 is elevation {}.", town.GetHeight(4t, 4t));
	conprint(0, u8"Elevation test: the tile at point 0.5×0.5 is elevation {}.", town.GetHeight(glm::vec3(0.50, 0.50, 10)));
	//town.GenerateNew(nullptr, 2, 2);
	//conprint(0, u8"Elevation test: on a new blank map, the tile at point 0.5×0.5 is elevation {}.", town.GetHeight(glm::vec3(0.50, 0.50, 10)));
	town.Load();
	int i = town.GetFlag("test:numTimesRan", 0);
	i++;
	town.SetFlag("test:numTimesRan", i);
	town.Save();

	return;
}
