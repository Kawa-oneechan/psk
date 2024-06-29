#include "SpecialK.h"
#include "DialogueBox.h"
#include "Player.h"

namespace SolBinds
{
	void Setup()
	{
		Sol.open_libraries(sol::lib::coroutine);

		Sol["print"] = [&](sol::variadic_args va)
		{
			console->Print(0, va[0]);
		};

		Sol["dialogue"] = sol::yielding([&](sol::variadic_args va)
		{
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
				//else va[1] is the speaker and va[2] is a style
				break;
			}

			//do that mutex thing
			dlgBox->Text(line, style);

			//ensure we can see the result
			console->visible = false;
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
			//"Catchphrase", &Villager::Catchphrase,
			//"Nickname", &Villager::Nickname
		);

		Sol["getVillager"] = [](sol::variadic_args va)
		{
			if (va.size() == 1)
			{
				std::shared_ptr<Villager> ret = nullptr;
				if (va[0].is<int>())
					ret = Database::Find(va[0].as<int>(), villagers);
				else if (va[0].is<std::string>())
					ret = Database::Find(va[0].as<std::string>(), villagers);
				if (ret == nullptr)
					conprint(1, "getVillager: could not find villager {}", va[0].as<std::string>());
				return ret;
			}
			conprint(1, "getVillager needs one argument, a hash or an ID.");
			return (std::shared_ptr<Villager>)nullptr;
		};
		Sol["getItem"] = [](sol::variadic_args va)
		{
			if (va.size() == 1)
			{
				std::shared_ptr<Item> ret = nullptr;
				if (va[0].is<int>())
					ret = Database::Find(va[0].as<int>(), items);
				else if (va[0].is<std::string>())
					ret = Database::Find(va[0].as<std::string>(), items);
				if (ret == nullptr)
					conprint(1, "getItem: could not find item {}", va[0].as<std::string>());
				return ret;
			}
			conprint(1, "getItem needs one argument, a hash or an ID.");
			return (std::shared_ptr<Item>)nullptr;
		};
	}
}
