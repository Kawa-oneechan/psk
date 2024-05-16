#include <cstdio>
#include <cmath>
#include <chrono>
#include <thread>
#include <conio.h>

#include "support/miniz.h"
#include "SpecialK.h"

static void prepForUTF8andSuch()
{
#ifdef _WIN32
	//Actually allow output in UTF8
	SetConsoleOutputCP(65001);
	setlocale(LC_ALL, ".UTF8");
#endif
#ifdef VTS
	//Completely optional, allow Virtual Terminal Sequences
	unsigned long mode = 0;
	GetConsoleMode(GetStdHandle(STDOUT), &mode);
	mode |= 4;
	SetConsoleMode(GetStdHandle(STDOUT), mode);

	fmt::print(REDWARNING u8" " UNDERLINE u8"HERE WE GO" NORMAL u8" " REDWARNING NORMAL u8"\n");
	fmt::print(u8"Is this \"ramune\" in kana? ⇒ \"" GREEN u8"ラムネ" NORMAL u8"\"\n");
	fmt::print(u8"árvíztűrő tükörfúrógép\n");
	fmt::print("\n");
#else
	fmt::print(u8"Is this \"ramune\" in kana? → \"ラムネ\"\n");
#endif
}

__declspec(noreturn)
void FatalError(const std::string& message)
{
#ifdef _CONSOLE
	fmt::print(REDWARNING "{}\nPress any key to exit.\n", message);
	_getch();
	exit(1);
#else
	wchar_t w[1024] = { 0 };
	MultiByteToWideChar(65001, 0, message.c_str(), -1, w, 1024);
	MessageBox(nullptr, w, L"Project Special K", 0x30);
	exit(1);
#endif
}

bool IDIsQualified(const std::string& id)
{
	//must have a : but not as the first character.
	return id.find(':') != std::string::npos && id[0] != ':';
}

void Qualify(std::string& id, const std::string& ns)
{
	if (id.substr(0, ns.length()) == ns)
		throw std::runtime_error(fmt::format("Qualify: cannot double-qualify \"{}\", already starts with \"{}\".", id, ns));
	id = ns + ':' + id;
}

void UnQualify(std::string& id)
{
	if (IDIsQualified(id))
		id = id.substr(id.find(':') + 1);
}

void testIDMangling()
{
	try
	{
		auto& fullyQualified = "ns:id"s;
		auto& unqualified = "id2"s;

		if (!IDIsQualified(fullyQualified))
			throw std::runtime_error(fmt::format("ID test: \"{}\" is NOT considered fully qualified.", fullyQualified));

		if (IDIsQualified(unqualified))
			throw std::runtime_error(fmt::format("ID test: \"{}\" IS considered fully qualified.", unqualified));

		Qualify(unqualified, "ns");
		if (unqualified != "ns:id2")
			throw std::runtime_error(fmt::format("ID test: \"{}\" should be \"ns:id2\"."));
		if (!IDIsQualified(unqualified))
			throw std::runtime_error(fmt::format("ID test: \"{}\" is NOT considered fully qualified.", unqualified));

		//Qualify(unqualified, "ns");
		//if (unqualified != "ns:id2")
		//	throw std::runtime_error(fmt::format("ID test: \"{}\" should be \"ns:id2\"."));

		UnQualify(unqualified);
		if (unqualified != "id2")
			throw std::runtime_error(fmt::format("ID test: \"{}\" should be \"ns:id2\"."));
		if (IDIsQualified(unqualified))
			throw std::runtime_error(fmt::format("ID test: \"{}\" IS considered fully qualified.", unqualified));

		fmt::print("ID test: if you reached this, we're golden.\n");
	}
	catch (std::runtime_error& e)
	{
		fmt::print(REDWARNING u8" {}" NORMAL u8"\n", e.what());
	}
}

void testCurrencies()
{
	auto ratesDoc = ReadJSON("currency.json");
	auto rates = std::map<std::string, float>();
	for (const auto& r : ratesDoc->AsObject())
		rates[r.first] = 1.0f / (float)r.second->AsNumber();

	auto inValue = 2;
	auto rate = (float)inValue / rates["USD"];
	auto outValue = std::roundf(rate * 100) / 100;
	//Proper answer as of 2024-05: $ 2.00 -> ¥ 300
	//Value given: ¥ 300

	inValue = 2; //-V1048
	rate = (float)inValue * rates["HUF"];
	outValue = std::roundf(rate * 100) / 100; //-V519
	//Proper answer as of 2024-05: ¥ 2 -> Ft 4.86
	//Value given: Ft 4.88
}

void testInventorySystems()
{
	//auto test = std::vector<InventoryItem*>();
	//test.reserve(10);
	//for (int i = 0; i < 10; i++)
	//	test.push_back(nullptr);
	//InventoryItem* test[10] = { nullptr };
	auto flipflops = new InventoryItem("acnh:flipflops/pink");
	thePlayer.GiveItem(new InventoryItem("acnh:snowysweater/black"));
	thePlayer.GiveItem(new InventoryItem("acnh:pleatherskirt"));
	thePlayer.GiveItem(flipflops);
	thePlayer.GiveItem(new InventoryItem("acnh:fronttietee/red"));
	thePlayer.GiveItem(new InventoryItem("acnh:denimcutoffs/navyblue"));
	thePlayer.GiveItem(new InventoryItem("acnh:palatialtankdress"));
	//test[0] = new InventoryItem("acnh:snowysweater/black");
	//test[1] = new InventoryItem("acnh:pleatherskirt");
	//test[2] = new InventoryItem("acnh:flipflops/pink");
	//test[3] = new InventoryItem("acnh:fronttietee/red");
	//test[4] = new InventoryItem("acnh:denimcutoffs");

	fmt::print("------Initial------\n");
	for (auto& i : thePlayer.OnHand)
		fmt::print("{} ", i != nullptr ? '[' + i->Name() + ']' : "*");
	fmt::print("\b\n");

	fmt::print("------Remove flipflops------\n");
	thePlayer.RemoveItem(flipflops);
	for (auto& i : thePlayer.OnHand)
		fmt::print("{} ", i != nullptr ? '[' + i->Name() + ']' : "*");
	fmt::print("\b\n");

	fmt::print("------Switch #3 and #5------\n");
	thePlayer.SwapItems(3, 5);
	for (auto& i : thePlayer.OnHand)
		fmt::print("{} ", i != nullptr ? '[' + i->Name() + ']' : "*");
	fmt::print("\b\n");

	fmt::print("------------\n");
}

int articlePlease;
Villager* lolly;

void delay(int ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void testVillagerGetting()
{
	lolly = (Villager*)Database::Find<Villager>("ac:cat18", &villagers);
	if (lolly == nullptr)
		FatalError("Could not load Lolly. Check the logs? Anyway, testing is over.");

	fmt::print("Villager getting test: {}, a {}, birthday on {}. ", lolly->Name(), lolly->Species(), lolly->Birthday());
	articlePlease = 0;
	fmt::print("Default outfit: {}\n", lolly->defaultOutfitID);
	fmt::print("Portrait: {}\n", lolly->portraitID);
	fmt::print("Manifesting...\n");
	lolly->Manifest();
	fmt::print("Current outfit: {}\n", lolly->Outfit->FullName());
	lolly->Depart();
	//fmt::print("Default outfit: {}\n", lolly->defaultOutfit->FullName());
	//fmt::print("Portrait: {}\n", lolly->portrait->FullName());
}

void testVillagerCatchphrases1()
{
	fmt::print("Villager catchphrase test: {}'s catchphrase is \"{}\".\n", lolly->Name(), lolly->Catchphrase());
	auto phrase = "my friend"s;
	fmt::print("Setting it to \"{}\".\n", phrase);
	lolly->Catchphrase(phrase);
}

void testVillagerCatchphrases2()
{
	fmt::print("Villager catchphrase retention test: {}'s catchphrase is \"{}\".\n", lolly->Name(), lolly->Catchphrase());
}

void testVillagerCatchphrases()
{
	testVillagerCatchphrases1();
	testVillagerCatchphrases2();
}

//int playerGender = 0; //for testing only
//std::string playerName = "Kawa"; //for testing only
std::string villagerID = "ac:sza"; //for testing only

void testConditionals()
{
	TextAdd(*ReadJSON("tests.json"));
	theVars["playerGender"] = textCondVar{ TextCondVarType::Integer, &thePlayer.Gender };
	theVars["playerName"] = textCondVar{ TextCondVarType::String, &thePlayer.Name };
	theVars["$mas"] = textCondVar{ TextCondVarType::ConstInt, 0 };
	theVars["$fem"] = textCondVar{ TextCondVarType::ConstInt, (void*)1 };
	theVars["$mnb"] = textCondVar{ TextCondVarType::ConstInt, (void*)2 };
	theVars["$fnb"] = textCondVar{ TextCondVarType::ConstInt, (void*)3 };
	auto result = TextGet("str:kun");
	fmt::print("Conditional test: with playerGender {}, result is \"{}\".\n", thePlayer.Gender, result);
	result = TextGet("condtest2");
	fmt::print("Conditional test: with playerName {}, result is \"{}\".\n", thePlayer.Name, result);
	fmt::print("Conditional test: changing name...\n");
	thePlayer.Name = "Lettie";
	result = TextGet("condtest2");
	fmt::print("Conditional test: with playerName {}, result is \"{}\".\n", thePlayer.Name, result);
}

#include <filesystem>
namespace fs = std::experimental::filesystem;
void testVillagerSerializing()
{
	//lolly->GivenItems.push_back(ResolveItem("ag:shinycatsuit", "topsfallback"));
	lolly->GivenItems.push_back(new InventoryItem("ag:shinycatsuit"));
	auto v = JSONObject();
	lolly->Serialize(v);
	auto val = JSONValue(v);
	auto stringified = JSON::Stringify(&val);
	//auto stringified = JSON::Stringify(&JSONValue(v));
	fmt::print("Serialization test: result was\n{}\n", stringified);

	auto ret = mz_zip_add_mem_to_archive_file_in_place("__save.zip", "lolly.json", stringified.c_str(), stringified.length(), nullptr, 0, MZ_BEST_COMPRESSION);
	if (ret)
	{
		//we made it through boys, NOW we handle renaming the new file.
		fs::remove("save.zip");
		fs::rename("__save.zip", "save.zip");
		//fs::copy_file("__save.zip", "save.zip");
		//fs::remove("__save.zip");
		//gotta make sure these both return true!
	}

	lolly->PickOutfit();
	fmt::print("Current outfit: {}\n", lolly->Outfit->FullName());
}

void testVillagerDeserializing()
{
	auto json = JSON::Parse("{\"catchphrase\":\"lover\",\"givenItems\":[\"ag:shinycatsuit/black\"],\"id\":\"ac:cat18\"}")->AsObject();
	auto id = json["id"]->AsString();
	auto* villager = (Villager*)Database::Find<Villager>(id, &villagers);
	villager->Deserialize((JSONObject&)json);
	fmt::print("Villager deserialization test: {}'s catchphrase is \"{}\".\n", villager->Name(), villager->Catchphrase());
	fmt::print("Villager deserialization test: {}'s first given item is \"{}\".\n", villager->Name(), villager->GivenItems[0]->FullName());
}

typedef enum
{
	Nothing, Opening, Writing, Delaying, WaitingForKey, Done
} DialogueBoxState;

class DialogueBox : public Tickable
{
private:
	std::string name;
	std::string* nameTag;
	std::string line;
	int delay;

	int cursorLine, cursorCol;
	size_t cursorPos;

	//temporary
	bool bold;

	#define MSBTParams const std::vector<std::string>& tags, int start, int len
	typedef void(DialogueBox::*MSBTFunc)(MSBTParams);

#pragma region Phase 1 MSBT
	void msbtStr(MSBTParams)
	{
		if (tags[1] == "...")
			line.replace(start, len, TextGet("str:fix:001"));
		else if (tags[1] == "player")
			line.replace(start, len, "Kawa");
		else if (tags[1] == "kun")
			line.replace(start, len, TextGet("str:kun"));
	}

	void msbtEllipses(MSBTParams)
	{
		auto fakeTags = std::vector<std::string>
		{
			"str", "..."
		};
		msbtStr(fakeTags, start, len);
	}

	std::map<std::string, MSBTFunc> msbtPhase1
	{
		{ "str", &DialogueBox::msbtStr },
		{ "...", &DialogueBox::msbtEllipses }
	};

	std::string& preprocess(std::string& input)
	{
		for (size_t i = 0; i < input.length(); i++)
		{
			auto msbtStart = input.find_first_of('<', i);
			if (msbtStart != std::string::npos)
			{
				msbtStart++;
				auto msbtEnd = input.find_first_of('>', msbtStart);
				i = msbtEnd;

				auto msbtWhole = input.substr(msbtStart, msbtEnd - msbtStart);
				//fmt::print("(MSBT: {})", msbtWhole);
				auto msbt = Split(msbtWhole, ':');
				auto func = msbtPhase1.find(msbt[0]);
				if (func != msbtPhase1.end())
				{
					std::invoke(func->second, this, msbt, (int)msbtStart - 1, (int)(msbtEnd - msbtStart) + 2);
					i = msbtStart; //-1 because we may have subbed in a new tag.
				}
			}
		}
		return input;
	}
#pragma endregion

#pragma region Phase 2 MSBT
	void msbtSize(MSBTParams)
	{
		bold = true;
	}

	void msbtSizeC(MSBTParams)
	{
		bold = false;
	}

	void msbtBreak(MSBTParams)
	{
		state = DialogueBoxState::WaitingForKey;
	}

	void msbtDelay(MSBTParams)
	{
		state = DialogueBoxState::Delaying;
		delay = std::stoi(tags[1]);
	}

	void msbtClear(MSBTParams)
	{
		//drawDialogueBox();
	}

	void msbtDummy(MSBTParams)
	{
		//
	}

	void msbtEnd(MSBTParams)
	{
		//dialogueForceEnd = true;
	}

	std::map<std::string, MSBTFunc> msbtPhase2
	{
		{ "size", &DialogueBox::msbtSize },
		{ "/size", &DialogueBox::msbtSizeC },
		{ "break", &DialogueBox::msbtBreak },
		{ "brk", &DialogueBox::msbtBreak },
		{ "delay", &DialogueBox::msbtDelay },
		{ "end", &DialogueBox::msbtEnd },
		{ "clr", &DialogueBox::msbtClear },
		{ "emote", &DialogueBox::msbtDummy },
	};
#pragma endregion

	#undef MSBTParams

public:
	DialogueBoxState state;
	DialogueBox()
	{
		name.clear();
		nameTag = nullptr;
		line.clear();
		delay = 0;
		cursorLine = 0;
		cursorCol = 0;
		cursorPos = 0;
		state = DialogueBoxState::Opening;
		bold = false;
	}

	void Start(std::string line, Villager* nameTagSource)
	{
		if (nameTagSource == nullptr)
		{
			name.clear();
			nameTag = nullptr;
		}
		else
		{
			name = nameTagSource->Name();
			nameTag = nameTagSource->NameTag;
		}

		this->line = preprocess(line);
		state = DialogueBoxState::Opening;
		cursorLine = 0;
		cursorCol = 0;
		cursorPos = 0;
	}

	void Tick(double dt)
	{
		if (state == DialogueBoxState::Opening)
		{
			fmt::print("\x1B[0m\x1B[1;1H"); //\x1B[2J
			fmt::print(u8"┌────────────────────────────────────────┐\n");
			fmt::print(u8"│                                        │\n");
			fmt::print(u8"│                                        │\n");
			fmt::print(u8"│                                        │\n");
			fmt::print(u8"└────────────────────────────────────────┘\n");

			if (nameTag != nullptr)
			{
				fmt::print("\x1B[1;3H");
				fmt::print("\x1B]4;5;rgb:{}/{}/{}\x07", nameTag[0].substr(1, 2), nameTag[0].substr(3, 2), nameTag[0].substr(5, 2));
				fmt::print("\x1B]4;13;rgb:{}/{}/{}\x07", nameTag[1].substr(1, 2), nameTag[1].substr(3, 2), nameTag[1].substr(5, 2));
				fmt::print("\x1B[45;95m");
				fmt::print("  {}  ", name);
			}

			fmt::print("\x1B[0m");
			state = DialogueBoxState::Writing;
		}
		else if (state == DialogueBoxState::Writing)
		{
			if (delay > 0)
			{
				delay -= (int)std::ceil(dt);
			}
			else
			{
				fmt::print("\x1B[0m");
				fmt::print("\x1B[{};{}H", cursorLine + 2, cursorCol + 3);
				auto ch = line[cursorPos];
				if (ch == '<')
				{
					auto msbtStart = ++cursorPos;
					auto msbtEnd = line.find_first_of('>', msbtStart);
					cursorPos = msbtEnd;

					auto msbtWhole = line.substr(msbtStart, msbtEnd - msbtStart);
					//fmt::print("(MSBT: {})", msbtWhole);
					auto msbt = Split(msbtWhole, ':');
					//if (msbt[0] == "end") break;
					auto func = msbtPhase2.find(msbt[0]);
					if (func != msbtPhase2.end())
						std::invoke(func->second, this, msbt, (int)msbtStart - 1, (int)(msbtEnd - msbtStart) + 2);
					else
						fmt::print("\x1B[5;60H <{}>?", msbt[0]);
				}
				else if (ch == '\n')
				{
					cursorLine++;
					cursorCol = 0;
					if (cursorLine == 3)
						state = DialogueBoxState::WaitingForKey;
				}
				else
				{
					if (bold) fmt::print(BOLD);
					fmt::print("{}", ch);
					cursorCol++;
					delay = 4;
				}
				cursorPos++;
				if (cursorPos >= line.length())
					state = DialogueBoxState::WaitingForKey;
			}
		}
		else if (state == DialogueBoxState::Delaying)
		{
			delay--;
			if (delay <= 0)
				state = DialogueBoxState::Writing;
		}
		else if (state == DialogueBoxState::WaitingForKey)
		{
			delay++;
			if (delay >= 60)
				delay = 0;
			if (delay == 30)
				fmt::print(u8"\x1B[5;38H\x1B[36m" u8"▼");
			else if (delay == 0)
				fmt::print(u8"\x1B[5;38H\x1B[0m" u8"──");

			auto key = _kbhit();
			if (key != 0)
			{
				_getch();
				fmt::print(u8"\x1B[5;38H\x1B[0m" u8"──");
				if (cursorPos >= line.length())
					state = DialogueBoxState::Done;
				else
					state = DialogueBoxState::Opening;
				cursorLine = 0;
				cursorCol = 0;
			}
		}

		auto states = std::vector<std::string>{ "Nothing", "Opening", "Writing", "Delaying", "Waiting", "Done" };
		fmt::print("\x1B[0m");
		fmt::print("\x1B[1;60H DialogueBox");
		fmt::print("\x1B[2;60H state {}      ", states[state]);
		fmt::print("\x1B[3;60H delay {}  ", delay);
		fmt::print("\x1B[4;60H pos   {}/{}  ", cursorPos, line.length());
		fmt::print("\x1B[5;60H dt    {}     ", dt);
	}
};

class GiftBalloonSpawner : Tickable
{
private:
	float latitude;
	bool goWest;
public:
	void Start()
	{
		latitude = 0;
	}

	void Tick(double dt)
	{
		latitude += (float)dt * 0.0025f;
		if (latitude > 64.0f)
		{
			latitude = 0;
			goWest = !goWest;
		}

		fmt::print("\x1B[0m");
		fmt::print("\x1B[10;1H GiftBalloonSpawner");
		fmt::print("\x1B[11;1H latitude {:.4}   ", latitude);
		fmt::print("\x1B[12;1H aim west {}   ", goWest);
	}
};

#include <ctime>

class DateTimePanel : Tickable
{
private:
	std::string shownTime, shownDate;
	int lastHour = -1;
	int lastMinute = -1;
	tm gm;

public:
	DateTimePanel()
	{
		auto now = time(nullptr);
		localtime_s(&gm, &now);
		Update();
	}

	void Update()
	{
		//24 hours, easy
		//shownTime = fmt::format("{:2}:{:02}", gm.tm_hour, gm.tm_min);

		//12 hours?
		auto h = gm.tm_hour;
		auto pm = h >= 12;
		if (h == 0) h += 12;
		else if (h > 12) h -= 12;

		shownTime = fmt::format("{:2}:{:02} {}", h, gm.tm_min, pm ? "PM" : "AM");
		
		auto wd = gm.tm_wday;
		if (wd == 0) wd = 7; //gm.tm_wday is 0-Sun to 6-Sat. We want 1-Mon to 7-Sun.

		//TODO : use "month:format".
		shownDate = fmt::format("{} {}, {}", TextGet(fmt::format("month:{}", gm.tm_mon + 1)), gm.tm_mday, TextGet(fmt::format("day:short:{}", wd)));

		if (lastHour == 4 && gm.tm_hour == 5)
		{
			fmt::print("\x1B[12;40H NEXT DAY            ");
			//trigger reset
		}
		if (gm.tm_hour != lastHour && gm.tm_min == 0)
		{
			lastHour = gm.tm_hour;
			fmt::print("\x1B[12;40H Ding dong~! {} now", lastHour);
			//trigger music
		}
	}

	void Tick(double dt)
	{
		auto now = time(nullptr);
		localtime_s(&gm, &now);
		if (lastMinute != gm.tm_min)
		{
			Update();
			lastMinute = gm.tm_min;
		}

		fmt::print("\x1B[10;40H DateTime");
		fmt::print("\x1B[11;40H {}, {}", shownTime, shownDate);
		if (gm.tm_min == 1)
			fmt::print("\x1B[12;40H                     ");
	}
};

void testDialogueAndMultiTasking()
{
	fmt::print("\x1B[2J");

	TextAdd(*ReadJSON("tests.json"));
	auto line = TextGet("dlg:sza:wack");

	auto villager = (Villager*)Database::Find<Villager>("ac:sza", &villagers);
	DialogueBox dlg;
	dlg.Start(line, villager);

	GiftBalloonSpawner gbs;
	gbs.Start();

	DateTimePanel dtp;

	int oldTime = 0;

	while (dlg.state != DialogueBoxState::Done)
	{
		int newTime = std::clock();
		int deltaTime = newTime - oldTime;
		oldTime = newTime;
		double dt = deltaTime;

		delay(1);
		dlg.Tick(dt);
		gbs.Tick(dt);
		dtp.Tick(dt);
	}
}

int main(int argc, char** argv)
{
	prepForUTF8andSuch();

#ifndef _CONSOLE
	for (int i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			if (argv[i] == "-console"s)
			{
				FILE* temp = nullptr;
				AllocConsole();
				freopen_s(&temp, "CONIN$", "r", stdin);
				freopen_s(&temp, "CONOUT$", "w", stdout);
				freopen_s(&temp, "CONOUT$", "w", stderr);
				prepForUTF8andSuch();
			}
		}
	}
#endif

	std::srand((unsigned int)std::time(nullptr));

	InitVFS();

	testIDMangling();
	testCurrencies();

	thePlayer.Name = "Kawa";
	thePlayer.Gender = Gender::BEnby;

	TextAdd(*ReadJSON("datetime.json"));
	TextAdd(*ReadJSON("fixedform.json"));
	Database::LoadGlobalStuff();

	testInventorySystems();

	testVillagerGetting();
	testVillagerCatchphrases();
	testConditionals();
	testVillagerSerializing();
	testVillagerDeserializing();
	testDialogueAndMultiTasking();

	{
		auto jock = Database::Find<::Personality>("jock", &personalities);
		auto sister = Database::Find<::Personality>("uchi", &personalities);

		Villager* starters[2];

		auto ret = std::vector<std::string>();
		ret.reserve(50);
		for (const auto& v : villagers)
		{
			if (v.personality == jock)
				ret.push_back(v.ID);
		}
		fmt::print("{} jocks to pick a starter from.\n", ret.size());
		starters[0] = (Villager*)Database::Find<::Villager>(ret[std::rand() % ret.size()].c_str(), &villagers);

		ret.clear();
		for (const auto& v : villagers)
		{
			if (v.personality == sister)
				ret.push_back(v.ID);
		}
		fmt::print("{} big sisters to pick a starter from.\n", ret.size());
		starters[1] = (Villager*)Database::Find<::Villager>(ret[std::rand() % ret.size()].c_str(), &villagers);

		fmt::print("Staring villagers: {} and {}.\n", starters[0]->Name(), starters[1]->Name());
	}

	fmt::print("\n\n\n\n\nPress any key.\n");
	_getch();
	return 0;
}

extern "C" int __stdcall WinMain(struct HINSTANCE__*, struct HINSTANCE__*, char*, int) { return main(__argc, __argv); }
