#include "SpecialK.h"

#ifdef DEBUG

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

//move this into Utilities
static tm _tm;
/* Returns a calendar date for things like "the fourth Friday in November".
*/
tm* GetNthWeekdayOfMonth(int month, int dayOfWeek, int howManyth)
{
	//C++17 doesn't have chrono calendar stuff yet, that's C++20.
	//So instead we're figuring this out the hard way.
	tm rightNow;
	time_t yesNow = std::time(nullptr);
	gmtime_s(&rightNow, &yesNow);
	int thisYear = rightNow.tm_year;

	tm novemberFirst = { 0 };
	novemberFirst.tm_year = thisYear;
	novemberFirst.tm_mon = month;
	novemberFirst.tm_mday = 1;
	novemberFirst.tm_hour = 12;
	std::mktime(&novemberFirst);
	time_t t = std::mktime(&novemberFirst);
	while (true)
	{
		tm here;
		gmtime_s(&here, &t);
		if (here.tm_wday == dayOfWeek)
		{
			t += ((60 * 60 * 24) * 7) * (howManyth - 1); //add a whole week for the amount of thursdays we want
			if (novemberFirst.tm_wday == dayOfWeek)
				t += (60 * 60 * 24) * 7; //add one more week if we *started* on a thursday
			break;
		}
		t += (60 * 60 * 24); //add one day
	}
	gmtime_s(&_tm, &t);
	return &_tm;
}

void RunTests()
{
	//test texture re-use
	auto textureCacheTest = Texture("ui/panels.png");

	TestInventorySystems();
	TestVillagerGetting();

	char ascTime[256] = { 0 };
	auto thanksGiving = GetNthWeekdayOfMonth(10, 4, 4);
	asctime_s(ascTime, thanksGiving);
	auto blackFriday = GetNthWeekdayOfMonth(10, 5, 4);
	asctime_s(ascTime, blackFriday);

	return;
}
#else
void RunTests()
{
	//Nothing in debug mode, bro.
}
#endif
