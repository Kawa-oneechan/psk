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

//Move this over to its own unit.
namespace NookCode
{
	static const char alphabet[]{ "ABCDEFGHIJKLMNOPQRSTUVWXYZ!@#&+-" };

	static inline unsigned char reverseBits(unsigned char b)
	{
		return unsigned char((b * 0x0202020202ULL & 0x010884422010ULL) % 1023);
	}

	static inline unsigned char rotateLeft(unsigned char b, int i)
	{
		while (i > 0)
		{
			unsigned char m = (b & 0x80) ? 1 : 0;
			b <<= 1;
			b |= m;
			i--;
		}
		return b;
	}

	static inline unsigned char rotateRight(unsigned char b, int i)
	{
		while (i > 0)
		{
			unsigned char m = (b & 0x01) ? 1 : 0;
			b >>= 1;
			b |= m << 7;
			i--;
		}
		return b;
	}

	static inline void swap(unsigned char* a, unsigned char* b)
	{
		auto c = *a;
		*a = *b;
		*b = c;
	}

	std::string Encode(std::array<unsigned char, 8>& d)
	{
		for (int i = 0; i < 5; i++)
			d[5] += d[i];

		for (int i = 0; i < 6; i++)
			d[i] = reverseBits(d[i]);

		swap(&d[0], &d[1]); //-V525 yes yes I know
		swap(&d[2], &d[3]);
		swap(&d[2], &d[4]);
		swap(&d[0], &d[3]);

		for (int i = 0; i < 6; i++)
			d[i] = rotateLeft(d[i], i + 1);

		auto v = *(unsigned long long*)&d;
		auto pv = v;
		pv ^= pv >> 1;
		pv ^= pv >> 2;
		pv = (pv & 0x1111111111111111UL) * 0x1111111111111111UL;
		unsigned char parity = (pv >> 60) & 1;

		unsigned char c[10]{ 0 };
		for (int i = 0; i < 10; i++)
		{
			c[i] = v & 31;
			v >>= 5;
		}
		c[9] |= parity << 4;

		std::string ret{ ".........." };
		for (int i = 0; i < 10; i++)
			ret[i] = alphabet[c[i]];

		return ret;
	}

	std::string Encode(hash itemHash, int variant, int pattern)
	{
		auto d = std::array<unsigned char, 8>();
		d[0] = (itemHash >>  0) & 0xFF;
		d[1] = (itemHash >>  8) & 0xFF;
		d[2] = (itemHash >> 16) & 0xFF;
		d[3] = (itemHash >> 24) & 0xFF;
		d[4] = (unsigned char)(variant  <<  4) | (pattern & 0xF);
		return Encode(d);
	}

	std::array<unsigned char, 8> Decode(const std::string& code)
	{
		std::array<unsigned char, 8> d;

		unsigned char c[10]{ 0 };
		for (int i = 0; i < 10; i++)
		{
			auto pos = strchr(alphabet, code[i]);
			if (pos == nullptr)
				throw std::runtime_error("Invalid character in NookCode.");
			c[i] = (unsigned char)(pos - alphabet);
		}

		//unsigned char parity = c[9] & 16;
		c[9] &= 7;

		auto v = 0ULL;
		for (int i = 0; i < 10; i++)
			v |= ((unsigned long long)c[i]) << (5 * i);

		for (int i = 0; i < 8; i++)
		{
			d[i] = v & 0xFF;
			v >>= 8;
		}

		for (int i = 0; i < 8; i++)
			d[i] = rotateRight(d[i], i + 1);

		swap(&d[0], &d[3]); //-V525
		swap(&d[2], &d[4]);
		swap(&d[2], &d[3]);
		swap(&d[0], &d[1]);

		for (int i = 0; i < 6; i++)
			d[i] = reverseBits(d[i]);

		unsigned char check = 0;
		for (int i = 0; i < 5; i++)
			check += d[i];
		if (check != d[5])
			throw std::runtime_error("NookCode checksum failed.");

		return d;
	}

	void Decode(const std::string& code, hash& itemHash, int& variant, int& pattern)
	{
		auto d = Decode(code);
		itemHash = d[0];
		itemHash |= d[1] << 8;
		itemHash |= d[2] << 16;
		itemHash |= d[3] << 24;
		variant = (d[4] >> 4) & 0xF;
		pattern = d[4] & 0xF;
	}
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
	auto textureCacheTest = Texture("ui/panels.png");

	TestInventorySystems();
	TestVillagerGetting();

	char ascTime[256] = { 0 };
	auto thanksGiving = GetNthWeekdayOfMonth(10, 4, 4);
	asctime_s(ascTime, thanksGiving);
	auto blackFriday = GetNthWeekdayOfMonth(10, 5, 4);
	asctime_s(ascTime, blackFriday);

	conprint(0, u8"Elevation test: the tile at tile 4×4 is elevation {}.", town.GetHeight(4t, 4t));
	conprint(0, u8"Elevation test: the tile at point 0.5×0.5 is elevation {}.", town.GetHeight(glm::vec3(0.50, 0.50, 10)));
	town.GenerateNew(nullptr, 2, 2);
	conprint(0, u8"Elevation test: on a new blank map, the tile at point 0.5×0.5 is elevation {}.", town.GetHeight(glm::vec3(0.50, 0.50, 10)));
	town.Load();
	int i = town.GetFlag("test:numTimesRan", 0);
	i++;
	town.SetFlag("test:numTimesRan", i);
	town.Save();

	return;
}
