#include <cstdio>
#include <cmath>
#include <chrono>
#include <thread>
#include <conio.h>
#include "format.h"


#include "VFS.h"
#include "Database.h"
#include "Species.h"
#include "Villager.h"
#include "Text.h"

extern "C"
{
#if 1
	//why bother including windows headers lol
	_declspec(dllimport) int __stdcall SetConsoleOutputCP(_In_ unsigned int wCodePageID);
	_declspec(dllimport) int __stdcall GetConsoleMode(_In_ void* hConsoleHandle, _Out_ unsigned long* lpMode);
	_declspec(dllimport) int __stdcall SetConsoleMode(_In_ void* hConsoleHandle, _In_ unsigned long dwMode);
	_declspec(dllimport) void* __stdcall GetStdHandle(_In_ unsigned long nStdHandle);
#define STDOUT ((unsigned long)-11)

#define REDWARNING "\x1b[91m" "\xE2\x9B\x94" "\x1B[0m"
#define BOLD "\x1B[1m"
#define UNDERLINE "\x1B[4m"
#define NORMAL "\x1B[0m"
#define GREEN "\x1B[92m"

	static void prepForUTF8andSuch()
	{
		//Actually allow output in UTF8
		SetConsoleOutputCP(65001);
		setlocale(LC_ALL, ".UTF8");

		//Completely optional, allow Virtual Terminal Sequences
		unsigned long mode = 0;
		GetConsoleMode(GetStdHandle(STDOUT), &mode);
		mode |= 4;
		SetConsoleMode(GetStdHandle(STDOUT), mode);

		printf(REDWARNING " " UNDERLINE "HERE WE GO" NORMAL " " REDWARNING NORMAL "\n");
		printf("Is this \"ramune\" in kana? --> \"" GREEN u8"ラムネ" NORMAL "\"\n");
		printf(u8"árvíztűrő tükörfúrógép\n");
		printf("\n");
	}
#else
	_declspec(dllimport) int __stdcall SetConsoleOutputCP(_In_ unsigned int wCodePageID);
	static void prepForUTF8andSuch()
	{
		SetConsoleOutputCP(65001);
		setlocale(LC_ALL, ".UTF8");
		printf(u8"Is this \"ramune\" in kana? --> \"ラムネ\"\n");
	}
#endif
}

void delay(int ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void testVillagerGetting()
{
	auto lolly = (Villager*)Database::Find<Villager>("ac:cat18", &villagers);
	fmt::print("Villager getting test: {}, a {}, birthday on {}.\n", lolly->Name(), lolly->Species(), lolly->Birthday());
}

void testVillagerCatchphrases1()
{
	auto lolly = (Villager*)Database::Find<Villager>("ac:cat18", &villagers);
	fmt::print("Villager catchphrase test: {}'s catchphrase is \"{}\".\n", lolly->Name(), lolly->Catchphrase());
	std::string phrase = "my friend";
	fmt::print("Setting it to \"{}\".\n", phrase);
	lolly->SetCatchphrase(phrase);
}

void testVillagerCatchphrases2()
{
	auto lolly = (Villager*)Database::Find<Villager>("ac:cat18", &villagers);
	fmt::print("Villager catchphrase retention test: {}'s catchphrase is \"{}\".\n", lolly->Name(), lolly->Catchphrase());
}

void testVillagerCatchphrases()
{
	testVillagerCatchphrases1();
	testVillagerCatchphrases2();
}

int playerGender = 0; //for testing only
std::string playerName = "Kawa"; //for testing only

void testConditionals()
{
	TextAdd(*ReadJSON("tests.json"));
	theVars["playerGender"] = textCondVar{ 0, &playerGender };
	theVars["playerName"] = textCondVar{ 1, &playerName };
	//{ "playerGender", &playerGender }
	auto result = TextGet("str:kun");
	fmt::print("Conditional test: with playerGender {}, result is \"{}\".\n", playerGender, result);
	result = TextGet("condtest2");
	fmt::print("Conditional test: with playerName {}, result is \"{}\".\n", playerName, result);
	fmt::print("Conditional test: changing name...\n");
	playerName = "Lettie";
	result = TextGet("condtest2");
	fmt::print("Conditional test: with playerName {}, result is \"{}\".\n", playerName, result);
}

void testMSBT()
{
	fmt::print("\n\nMSBT test:\n");
	fmt::print("---------------\n");
	TextAdd(*ReadJSON("tests.json"));
	//auto line = TextGet("dlg:sza:wack");
	auto line = TextGet("dlg:sza:insertions");

	//Preprocess to handle substitutions.
	for (int i = 0; i < line.length(); i++)
	{
		auto msbtStart = line.find_first_of('<', i);
		if (msbtStart != std::string::npos)
		{
			msbtStart++;
			auto msbtEnd = line.find_first_of('>', msbtStart);
			i = msbtEnd;

			auto msbtWhole = line.substr(msbtStart, msbtEnd - msbtStart);
			//fmt::print("(MSBT: {})", msbtWhole);
			auto msbt = Split(msbtWhole, ':');
			if (msbt[0] == "str")
			{
				if (msbt[1] == "player")
				{
					line.replace(msbtStart - 1, msbtEnd - msbtStart + 2, "Kawa");
					i = msbtStart - 1; //-1 because we may have subbed in a new tag.
				}
				else if (msbt[1] == "kun")
				{
					line.replace(msbtStart - 1, msbtEnd - msbtStart + 2, TextGet("str:kun"));
					i = msbtStart - 1;
				}
			}
		}
	}

	fmt::print("This test involves screen clearing. Press any key when ready.");
	_getch();

	//Reset, clear, and hide cursor.
	fmt::print("\x1B[1;1H\x1B[2J\x1B[?25l");

	//Reprocess to actually handle things like delays and colors.
	for (int i = 0; i < line.length(); i++)
	{
		if (line[i] == '<')
		{
			auto msbtStart = ++i;
			auto msbtEnd = line.find_first_of('>', msbtStart);
			i = msbtEnd;

			auto msbtWhole = line.substr(msbtStart, msbtEnd - msbtStart);
			//fmt::print("(MSBT: {})", msbtWhole);
			auto msbt = Split(msbtWhole, ':');
			if (msbt[0] == "size")
				fmt::print(BOLD);
			else if (msbt[0] == "/size")
				fmt::print(NORMAL);
			else if (msbt[0] == "break")
			{
				fmt::print(u8"\t▼");
				_getch();
				fmt::print("\x1B[1;1H\x1B[2J");
			}
			else if (msbt[0] == "delay")
				delay(std::stoi(msbt[1]));
			else if (msbt[0] == "clr")
				fmt::print("\x1B[1;1H\x1B[2J");
			else if (msbt[0] == "end")
			{
				break;
			}
			else
				fmt::print("<{}>", msbt[0]);
		}
		else
		{
			fmt::print("{}", line[i]);
			delay(20);
		}
	}
	//show cursor again
	fmt::print("\x1B[?25l");
	fmt::print("(MSBT: end of string)\n");
}

int main()
{
	prepForUTF8andSuch();

	InitVFS();

	TextAdd(*ReadJSON("datetime.json"));
	Database::LoadGlobalStuff();

	//testVillagerGetting();
	//testVillagerCatchphrases();
	testConditionals();
	//testMSBT();

	printf("\n\n\n\n\nPress any key.\n");
	_getch();
	return 0;
}
