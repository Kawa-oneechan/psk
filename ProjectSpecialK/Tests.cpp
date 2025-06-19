#include "SpecialK.h"
#include "Town.h"
#include "TextUtils.h"
#include "NookCode.h"

#include <stb_image.h>
#include <stb_image_write.h>
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

	conprint(0, "Villager getting test: {}, a {}, birthday on {}.", farrah->Name(), StripBJTS(farrah->Species()), farrah->Birthday());
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


int GetLetterScore(const std::string& text, bool noCapitals)
{
	auto trigrams = VFS::ReadString(fmt::format("mailcheck/trigrams_{}.txt", Text::GetLangCode()));
	int score = 0;
	
	rune ch;
	size_t size;

	auto find = [&](std::string haystack, rune pct, size_t pos)
	{
		rune ch2;
		size_t size2;
		while (pos < haystack.length())
		{
			std::tie(ch2, size2) = GetChar(haystack, pos);
			if (ch2 == pct)
				return pos;
			pos += size2;
		}
		return std::string::npos;
	};

	//Check A: punctuation and capital letters.
	{
		auto lastPos = text.length() - 1;
		while ((text[lastPos] & 0xC0) == 0x80)
			lastPos--;

		std::tie(ch, size) = GetChar(text, lastPos);

		if (ch == '.' || ch == '!' || ch == '?' || ch == 0x3002 || ch == 0xFF01 || ch == 0xFF0E || ch == 0xFF1F)
		{
			score += 20;
		}

		//Check this for correctness.

		auto checkFor = [&score, text, &find](rune pct)
		{
			size_t pos = find(text, pct, 0);
			while (pos != std::string::npos)
			{
				size_t size;
				if (pos != 0)
				{
					rune ch;
					std::tie(ch, size) = GetChar(text, pos);
					pos += size;
				}
				if (pos + 3 >= text.length())
					break;
				bool yea = false;
				size_t j = 0;
				for (int i = 0; i < 3; i++)
				{
					rune ch2;
					std::tie(ch2, size) = GetChar(text, pos + j);
					j += size;
					if (std::isupper(ch2))
					{
						score += 10;
						yea = true;
						break;
					}
				}
				if (!yea)
					score -= 10;
				pos = find(text, pct, pos);
				if (pos == 0)
					pos++;
			}
		};

		checkFor('.');
		checkFor('!');
		checkFor('?');
		checkFor(0x3002);
		checkFor(0xFF01);
		checkFor(0xFF0E);
		checkFor(0xFF1F);
	}

	//Check B: trigrams
	{
		size_t pos = 0;
		int trisFound = 0;

		std::tie(ch, size) = GetChar(text, 0);
		while (std::isblank(ch))
		{
			pos += size;
			std::tie(ch, size) = GetChar(text, pos);
		}
		size = 0;

		while (pos != std::string::npos)
		{
			if (pos != 0)
				pos++;
			if (pos + 3 >= text.length())
				break;

			//auto tri = text.substr(pos, 3);
			std::string tri{ "" };
			size_t poop;

			std::tie(ch, poop) = GetChar(text, pos);
			AppendChar(tri, ch);
			pos += poop;
			std::tie(ch, poop) = GetChar(text, pos);
			AppendChar(tri, ch);
			pos += poop;
			std::tie(ch, poop) = GetChar(text, pos);
			AppendChar(tri, ch);
			pos += poop;

			if (poop == 1)
				StringToLower(tri);

			auto triPos = trigrams.find(tri);
			if (triPos != std::string::npos)
				trisFound++;

			if (poop == 1)
			{
				pos = find(text, ' ', pos);
				if (pos != std::string::npos)
					pos++;
			}
		}
		score += trisFound * 3;
	}

	//Check C: first letter is a capital
	if (!noCapitals)
	{
		for (size_t i = 0; i < text.length(); i += size)
		{
			//auto c = text[i];
			std::tie(ch, size) = GetChar(text, i);
			if (std::isblank(ch))
			{
				continue;
			}
			if (std::isupper(ch))
				score += 20;
			else if (std::islower(ch))
				score -= 10;
			break;
		}
	}

	//Check D: repeated characters
	{
		for (size_t i = 0; i < text.length(); i += size)
		{
			//auto c = text[i];
			std::tie(ch, size) = GetChar(text, i);
			auto a = 0;
			rune ch2;
			size_t size2;
			for (size_t j = i + 1; j < text.length() && j < i + 3; j += size2, i += size2)
			{
				std::tie(ch2, size2) = GetChar(text, j);
				if (ch2 == ch)
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
		for (size_t i = 0; i < text.length(); i += size)
		{
			//auto c = text[i];
			std::tie(ch, size) = GetChar(text, i);
			if (std::isblank(ch))
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
			//if (first.find('.') == std::string::npos && first.find('!') == std::string::npos && first.find('?') == std::string::npos)
			if (find(first, '.', 0) == std::string::npos &&
				find(first, '!', 0) == std::string::npos &&
				find(first, '?', 0) == std::string::npos &&
				find(first, 0x3002, 0) == std::string::npos &&
				find(first, 0xFF01, 0) == std::string::npos &&
				find(first, 0xFF0E, 0) == std::string::npos &&
				find(first, 0xFF1F, 0) == std::string::npos
				//y'know what? commas.
				&& find(first, ',', 0) == std::string::npos
				)
				score -= 150;
		}
	}

	//Check G: at least one space per 32 character cluster
	{
		size_t i = 0;
		while (i < text.length())
		{
			//size_t l = 32;
			//if (i + l > text.length())
			//	l = text.length() - i;
			//auto chunk = text.substr(i, l);
			std::string chunk{ "" };
			size_t pos = i;
			for (size_t j = 0; j < 32 && pos < text.length(); j++, pos += size)
			{
				std::tie(ch, size) = GetChar(text, pos);
				AppendChar(chunk, ch);
			}

			if (find(chunk, ' ', 0) == std::string::npos &&
				find(chunk, 0x3000, 0) == std::string::npos
				)
				score -= 20;

			i = pos;
		}
	}

	return score;
}


void RunTests()
{
	//GetLetterScore(u8"Hello friend Bob. How are you? See ya!"); //should be 101, matches Hunter R's tool.
	//GetLetterScore(u8"こんにちは、ボブさん。元気ですか？またね！", true); //should be 40, Hunter R's tool doesn't support non-US.
	//GetLetterScore("L!L!L!L!L!L! L!L!L!L!L!L!L! L!L!L!L!L!L!L! L!L!L!L!L!L! L!L!L!L!L!L!L! L!L!L!L!L!L!L!L!L!L!L!L! L!L!L!L!L!L!L!L!L! L!L!L!L! L!L!L!L! L!L!L! L!L!L! L!L!L!"); //should be 710, Hunter R says 746.
	//GetLetterScore("!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i!i"); //should be -820, Hunter R says -807

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

	conprint(0, u8"Elevation test: the tile at tile 4×4 is elevation {}.", town->GetHeight(4t, 4t));
	conprint(0, u8"Elevation test: the tile at point 0.5×0.5 is elevation {}.", town->GetHeight(glm::vec3(0.50, 0.50, 10)));
	//town.GenerateNew(nullptr, 2, 2);
	//conprint(0, u8"Elevation test: on a new blank map, the tile at point 0.5×0.5 is elevation {}.", town.GetHeight(glm::vec3(0.50, 0.50, 10)));
	
	/*
	town->Load();
	int i = town->GetFlag("test:numTimesRan", 0);
	i++;
	town->SetFlag("test:numTimesRan", i);
	town->Save();
	*/

	return;
}
