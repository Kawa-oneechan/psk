#include "SpecialK.h"
#include "DialogueBox.h"
#include "Player.h"
#include "MusicManager.h"
#include "NookCode.h"

namespace SolBinds
{
	void Setup()
	{
		Sol.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::string, sol::lib::math);

		//Remove some base stuff
		Sol["dofile"] = nullptr;
		Sol["load"] = nullptr;
		Sol["loadfile"] = nullptr;
		Sol["loadstring"] = nullptr;

		Sol["print"] = [&](sol::variadic_args va)
		{
			console->Print(0, va[0]);
		};

		Sol["dialogue"] = sol::yielding([&](sol::variadic_args va)
		{
			VillagerP speaker = nullptr;
			int style = 0;
			std::string line;
			switch (va.size())
			{
			case 0:
				line = "[[Forgot to specify a line]]";
				break;
			case 1:
				line = va[0].as<std::string>();
				break;
			case 2:
				line = va[0].as<std::string>();
				if (va[1].is<int>())
					style = va[1].as<int>();
				else
				{
					if (va[1].is<VillagerP>())
						speaker = va[1].as<VillagerP>();
					else if (va[1].is<std::string>())
						speaker = Database::Find<Villager>(va[1].as<std::string>(), villagers);
					style = va[2].as<int>();
				}
				break;
			}

			dlgBox->Text(line, style);

			console->Close();
		});

		Sol.new_usertype<Player>(
			"__Player",
			sol::constructors<Player()>(),
			"Name", sol::readonly(&Player::Name),
			"Gender", &Player::Gender,
			"Bells", &Player::Bells
		);
		Sol["Player"] = &thePlayer;

		Sol["PlayerBag"] = sol::new_table();
		auto bag = Sol["PlayerBag"];
		bag["HasInventoryRoom"] = [&]() { return thePlayer.HasInventoryRoom(); };
		bag["SwapItems"] = [&](int from, int to) { thePlayer.SwapItems(from, to); };
		bag["RemoveItem"] = [&](int slot) { thePlayer.RemoveItem(slot); };
		bag["ConsumeItem"] = [&](int slot) { thePlayer.ConsumeItem(slot); };

		Sol.new_usertype<Villager>(
			"__Villager",
			"Name", sol::property(&Villager::Name),
			"Species", &Villager::Species
		);

		Sol["getVillager"] = [](sol::variadic_args va)
		{
			if (va.size() == 1)
			{
				VillagerP ret = nullptr;
				if (va[0].is<int>())
					ret = Database::Find(va[0].as<int>(), villagers);
				else if (va[0].is<std::string>())
					ret = Database::Find(va[0].as<std::string>(), villagers);
				if (!ret)
					conprint(1, "getVillager: could not find villager {}", va[0].as<std::string>());
				return ret;
			}
			conprint(1, "getVillager needs one argument, a hash or an ID.");
			return (VillagerP)nullptr;
		};
		Sol["getItem"] = [](sol::variadic_args va)
		{
			if (va.size() == 1)
			{
				ItemP ret = nullptr;
				if (va[0].is<int>())
					ret = Database::Find(va[0].as<int>(), items);
				else if (va[0].is<std::string>())
					ret = Database::Find(va[0].as<std::string>(), items);
				if (!ret)
					conprint(1, "getItem: could not find item {}", va[0].as<std::string>());
				return ret;
			}
			conprint(1, "getItem needs one argument, a hash or an ID.");
			return (ItemP)nullptr;
		};

		Sol["music"] = [](sol::variadic_args va)
		{
			if (va.size() == 1)
				musicManager->Play(va[0].as<std::string>());
			else if (va.size() == 2)
				musicManager->Play(va[0].as<std::string>(), va[1].as<bool>());
			console->Close();
		};

		Sol["decodeNookCode"] = [](const std::string& code)
		{
			hash itemHash;
			int variant, pattern;
			Sol["nookName"] = "XXX";
			NookCode::Decode(code, itemHash, variant, pattern);
			if (itemHash == (hash)-1)
				return 0; //Invalid characters in NookCode.
			if (itemHash == (hash)-2)
				return 1; //Checksum mismatch.
			//See if this identifies an item (despite the itemHash name)
			{
				auto item = Database::Find(itemHash, items);
				if (item)
				{
					//TODO: check if we already have this item.
					//TODO: put this item in the delivery queue for tomorrow
					Sol["nookName"] = item->Name();
					return 3; //Item will be delivered.
				}
			}
			{
				auto villager = Database::Find(itemHash, villagers);
				if (villager)
				{
					//TODO: check if this villager already lives here.
					//TODO: put this villager on the move-in queue
					Sol["nookName"] = villager->Name();
					return 5; //Villager will move in.
				}
			}
			return 2; //Valid NookCode, but unknown hash.
		};
	}
}
