#include "OptionsMenu.h"
#include "engine/Text.h"
#include "engine/Cursor.h"
#include "engine/TextUtils.h"
#include "Utilities.h"

extern bool botherColliding;

OptionsMenu::OptionsMenu()
{
	Build();
	Translate();

	stack.push(&options);
	items = stack.top();

	for (const auto& s : ::species)
	{
		if (!s->FilterAs.empty()) continue;
		speciesPreviews.push_back(new Texture("ui/species/" + s->ID + ".png"));
	}

	UpdateButtonGuide();
}

void OptionsMenu::Translate()
{
	speciesText = Text::Get("menu:options:content:species:help");
	options.Translate();
}

void OptionsMenu::Build()
{
	auto minutes = [&](DoomMenuItem* i)
	{
		return fmt::format(i->formatText, i->selection);
	};
	auto percent = [&](DoomMenuItem* i)
	{
		return fmt::format("{}%", i->selection);
	};
	auto speciesDrawer = [&](DoomMenuPage*, DoomMenuItem* i)
	{
		Sprite::DrawText(1, speciesText, glm::vec2(width * 0.6f, height * 0.4f) * scale, glm::vec4(1), 75.0f);
		if (i->type == DoomMenuItem::Type::Checkbox)
		{
			Sprite::DrawSprite(*speciesPreviews[highlight], glm::vec2((width * 0.5f) - (speciesPreviews[0]->width * 0.5f), (height * 0.5f) - (speciesPreviews[0]->height * 0.5f)) * scale);
		}
	};

	static const Language opt2lan[] = { Language::USen, Language::JPja, Language::EUde, Language::EUes, Language::EUfr, Language::EUit, Language::EUhu, Language::EUnl, Language::EUen };
	static const int lan2opt[] = {
		0, 3, 4,
		1, 0, 0, 0,
		2, 8, 3, 4, 5, 7, 0, 6 };

	options.items.clear();
	content.items.clear();
	keybinds.items.clear();
	volume.items.clear();

	options.headerKey = "menu:options:head";
	{
		options.items.push_back(new DoomMenuItem("menu:options:content", &content));
		options.items.push_back(new DoomMenuItem("menu:options:keybinds", &keybinds));
		options.items.push_back(new DoomMenuItem("menu:options:volume", &volume));
		options.items.push_back(new DoomMenuItem("menu:options:language", lan2opt[(int)gameLang],
		{
			"menu:options:language:en",
			"menu:options:language:jp",
			"menu:options:language:de",
			"menu:options:language:es",
			"menu:options:language:fr",
			"menu:options:language:it",
			"menu:options:language:hu",
			"menu:options:language:nl",
			//"menu:options:language:uk"
		},
		[&](DoomMenuItem*i)
		{
			gameLang = opt2lan[i->selection];
			UI::settings.as_object()["language"] = Text::GetLangCode(gameLang);
			Translate();
			items = &options;
			//dlgBox->Text(Text::Get("menu:options:language:taunt"), Database::Find<Villager>("psk:cat00", villagers));
		}
		));
		options.items.push_back(new DoomMenuItem("menu:options:continuefrom", UI::settings.as_object()["continue"].as_integer(),
		{
			"menu:options:continuefrom:0",
			"menu:options:continuefrom:1",
			"menu:options:continuefrom:2",
			"menu:options:continuefrom:3",
		},
		[&](DoomMenuItem*i) { UI::settings.as_object()["continue"] = i->selection; }
		));
		options.items.push_back(new DoomMenuItem("menu:options:speech", UI::settings.as_object()["speech"].as_integer(),
		{
			"menu:options:speech:0",
			"menu:options:speech:1",
			"menu:options:speech:2",
		},
		[&](DoomMenuItem*i) { UI::settings.as_object()["speech"] = i->selection; }
		));
		options.items.push_back(new DoomMenuItem("menu:options:pingrate", 2, 60, UI::settings.as_object()["pingRate"].as_integer(), 1, minutes,
			[&](DoomMenuItem*i) { UI::settings.as_object()["pingRate"] = i->selection; }
		));
		options.items.push_back(new DoomMenuItem("menu:options:balloonchance", 10, 60, UI::settings.as_object()["balloonChance"].as_integer(), 5, percent,
			[&](DoomMenuItem*i) { UI::settings.as_object()["balloonChance"] = i->selection; }
		));
		options.items.push_back(new DoomMenuItem("menu:options:bothercolliding", botherColliding,
			[&](DoomMenuItem*) { botherColliding = !botherColliding; }
		));
		options.items.push_back(new DoomMenuItem("menu:options:cursorscale", 50, 150, UI::settings.as_object()["cursorScale"].as_integer(), 10, percent,
			[&](DoomMenuItem*i)
		{
			cursor->SetScale(i->selection);
			UI::settings.as_object()["cursorScale"] = i->selection;
		}
		));
	}

	keybinds.headerKey = "menu:options:head:keybinds";
	{
		for (int i = 0; i < NumKeyBinds; i++)
		{
			auto f = fmt::format("menu:options:keybinds:{}", i);
			keybinds.items.push_back(new DoomMenuItem(f, (Binds)i));
		}
		keybinds.items.push_back(new DoomMenuItem("menu:options:keybinds:reset", [&](DoomMenuItem*i)
		{
			i;
			for (int j = 0; j < NumKeyBinds; j++)
			{
				Inputs.Keys[j].ScanCode = glfwGetKeyScancode(DefaultInputBindings[j]);
				Inputs.Keys[j].GamepadButton = DefaultInputGamepadBindings[j];
				Inputs.Keys[j].Name = GetKeyName(Inputs.Keys[j].ScanCode);
			}
		}));
	}

	content.headerKey = "menu:options:head:content";
	{
		species.items.clear();
		species.headerKey = "menu:options:head:content"; //-V691 "content" was not the imposter.
		species.subKey = "menu:options:content:species";
		for (const auto& s : ::species)
		{
			auto f = "filter:species:" + s->ID;
			if (!s->FilterAs.empty()) continue;

			if (Database::Filters.find(f) == Database::Filters.end())
				Database::Filters[f] = true;

			species.items.push_back(new DoomMenuItem((std::string&)f, Database::Filters[f],
				[&, f](DoomMenuItem*i)
			{
				Database::Filters[f] = i->selection > 0;
				auto s = UI::settings.as_object()["contentFilters"].as_object(); //-V836 can't be helped for now
				s[f] = Database::Filters[f];
				//s.insert_or_assign(f, Database::Filters[f]);
				//UI::settings.as_object()["contentFilters"] = s;
			}
			));
		}
		species.DrawSpecial = speciesDrawer;

		content.items.push_back(new DoomMenuItem("menu:options:content:species", &species));

		for (const auto& fc : Database::FilterCategories)
		{
			auto fck = fc.first;

			auto fcpage = new DoomMenuPage("menu:options:head:content", fck);
			for (const auto& f : fc.second)
			{

				fcpage->items.push_back(new DoomMenuItem((std::string&)f, Database::Filters[f],
					[&, f](DoomMenuItem*i)
				{
					Database::Filters[f] = i->selection > 0;
					auto s = UI::settings.as_object()["contentFilters"].as_object(); //-V836 can't be helped for now
					s[f] = Database::Filters[f];
					//s.insert_or_assign(f, Database::Filters[f]);
					//UI::settings.as_object()["contentFilters"] = s;
				}
				));
			}

			//TODO: Description field

			content.items.push_back(new DoomMenuItem(fck, fcpage));
		}
	}


	{
		volume.headerKey = "menu:options:head:volume";
		volume.items.push_back(new DoomMenuItem("menu:options:volume:music", 0, 100, (int)(Audio::MusicVolume * 100), 10, percent,
			[&](DoomMenuItem*i) { Audio::MusicVolume = i->selection / 100.0f; }
		));
		volume.items.push_back(new DoomMenuItem("menu:options:volume:ambience", 0, 100, (int)(Audio::AmbientVolume * 100), 10, percent,
			[&](DoomMenuItem*i) { Audio::AmbientVolume = i->selection / 100.0f; }
		));
		volume.items.push_back(new DoomMenuItem("menu:options:volume:sfx", 0, 100, (int)(Audio::SoundVolume * 100), 10, percent,
			[&](DoomMenuItem*i) { Audio::SoundVolume = i->selection / 100.0f; }
		));
		volume.items.push_back(new DoomMenuItem("menu:options:volume:speech", 0, 100, (int)(Audio::SpeechVolume * 100), 10, percent,
			[&](DoomMenuItem*i) { Audio::SpeechVolume = i->selection / 100.0f; }
		));
	}
}

bool OptionsMenu::Scancode(unsigned int scancode)
{
	//Eat shifts, controls, alts, and logo. NEVER accept these as binds.
	if (scancode == 0x2A || scancode == 0x36 || scancode == 0x1D || scancode == 0x11D || scancode == 0x38 || scancode == 0x138 || scancode == 0x15B)
		return false;

	if (remapping == -1)
		return false;

	if (remapBounce)
	{
		if (!(Inputs.KeyDown(Binds::Accept) || Inputs.MouseLeft))
			remapBounce = false;
		return false;
	}

	auto item = items->items[remapping];
	Inputs.Keys[item->selection].ScanCode = scancode;
	Inputs.Keys[item->selection].Name = GetKeyName(scancode);

	remapping = -1;
	Inputs.Clear();
	return true;
}