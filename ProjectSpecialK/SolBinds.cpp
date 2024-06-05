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
			"Player",
			sol::constructors<Player()>(),
			"Name", &Player::Name,
			"Gender", &Player::Gender,
			"Bells", &Player::Bells
			//Can't fit all of them in here?
			//"HasItemRoom", &Player::HasInventoryRoom,
			//"GiveItem", &Player::GiveItem,
			//"SwapItems", &Player::SwapItems,
			//"RemoveItem", &Player::RemoveItem,
			//"ConsumeItem", &Player::ConsumeItem
		);
		Sol["player"] = &thePlayer;

		Sol.new_usertype<Villager>(
			"Villager",
			"Name", &Villager::Name,
			"Species", &Villager::Species
			//"Catchphrase", &Villager::Catchphrase,
			//"Nickname", &Villager::Nickname
		);
		Sol["getVillager"] = [&](sol::variadic_args va)
		{
			return (Villager*)Database::Find(va[0].as<std::string>(), &villagers);
		};
	}
}
