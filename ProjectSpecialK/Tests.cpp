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

void RunTests()
{
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
