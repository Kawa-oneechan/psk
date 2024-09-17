#include "DoomMenu.h"
#include "InputsMap.h"
#include "Cursor.h"
#include "DialogueBox.h"

DoomMenuItem::DoomMenuItem(const std::string& cap, int val, std::initializer_list<std::string> opts, std::function<void(DoomMenuItem*)> chg = nullptr) : key(cap), type(Type::Options), selection(val), change(chg), page(nullptr)
{
	for (auto i : opts)
		optionKeys.emplace_back(i);
}

void DoomMenuItem::Translate()
{
	caption = Text::Get(key);
	auto k = key + ":desc";
	description = Text::Get(k);
	k = key + ":fmt";
	formatText = Text::Get(k);
	if (type == Type::Options)
	{
		options.clear();
		for (auto& i : optionKeys)
			options.emplace_back(Text::Get(i));
	}
	if (type == Type::Page && page != nullptr)
	{
		page->Translate();
	}
}

void DoomMenuItem::Beep()
{
	switch (type)
	{
	case Type::Slider:
	{
		auto beep = generalSounds["ui"]["selectShort"];

		auto ccur = clamp(selection, minVal, maxVal) - minVal;
		auto panpot = -1.0f + ((ccur / (float)(maxVal - minVal)) * 2.0f);

		beep->Play(true);
		beep->SetPan(panpot);
		break;
	}
	case Type::Checkbox:
	{
		generalSounds["ui"]["checkSmall"]->Play(true);
		break;
	}
	}
}

void DoomMenuPage::Translate()
{
	if (!headerKey.empty()) header = Text::Get(headerKey);
	if (!subKey.empty()) subheader = Text::Get(subKey);

	for (auto i : items)
	{
		i->Translate();
	}
}

void DoomMenu::Build()
{
	auto minutes = [&](DoomMenuItem* i)
	{
		return fmt::format(i->formatText, i->selection);
	};
	auto percent = [&](DoomMenuItem* i)
	{
		return fmt::format("{}%", i->selection);
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
			UI::settings["language"] = new JSONValue(i->selection);
			Translate();
			items = &options;
			dlgBox->Text(Text::Get("menu:options:language:taunt"), Database::Find<Villager>("psk:cat00", villagers));
		}
		));
		options.items.push_back(new DoomMenuItem("menu:options:continuefrom", UI::settings["continue"]->AsInteger(),
		{
			"menu:options:continuefrom:0",
			"menu:options:continuefrom:1",
			"menu:options:continuefrom:2",
			"menu:options:continuefrom:3",
		},
		[&](DoomMenuItem*i) { UI::settings["continue"] = new JSONValue(i->selection); }
		));
		options.items.push_back(new DoomMenuItem("menu:options:speech", UI::settings["speech"]->AsInteger(),
		{
			"menu:options:speech:0",
			"menu:options:speech:1",
			"menu:options:speech:2",
		},
		[&](DoomMenuItem*i) { UI::settings["speech"] = new JSONValue(i->selection); }
		));
		options.items.push_back(new DoomMenuItem("menu:options:pingrate", 2, 60, UI::settings["pingRate"]->AsInteger(), 1, minutes,
			[&](DoomMenuItem*i) { UI::settings["pingRate"] = new JSONValue(i->selection); }
		));
		options.items.push_back(new DoomMenuItem("menu:options:balloonchance", 10, 60, UI::settings["balloonChance"]->AsInteger(), 5, percent,
			[&](DoomMenuItem*i) { UI::settings["balloonChance"] = new JSONValue(i->selection); }
		));
		options.items.push_back(new DoomMenuItem("menu:options:cursorscale", 50, 150, UI::settings["cursorScale"]->AsInteger(), 10, percent,
			[&](DoomMenuItem*i)
		{
			cursor->SetScale(i->selection);
			UI::settings["cursorScale"] = new JSONValue(i->selection);
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
				auto s = UI::settings["contentFilters"]->AsObject(); //-V836 can't be helped for now
				s.insert_or_assign(f, new JSONValue(Database::Filters[f]));
				UI::settings["contentFilters"] = new JSONValue(s);
			}
			));
		}

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
					auto s = UI::settings["contentFilters"]->AsObject(); //-V836 can't be helped for now
					s.insert_or_assign(f, new JSONValue(Database::Filters[f]));
					UI::settings["contentFilters"] = new JSONValue(s);
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

void DoomMenu::Translate()
{
	speciesText = Text::Get("menu:options:content:species:help");
	options.Translate();
}

void DoomMenu::UpdateButtonGuide()
{
	switch(items->items[highlight]->type)
	{
	case DoomMenuItem::Type::KeyBind:
		buttonGuide.SetButtons({
			fmt::format(u8"!{} Assign", GamepadPUAMap[Inputs.Keys[(int)Binds::Accept].GamepadButton]),
			fmt::format(u8"{} Back", GamepadPUAMap[Inputs.Keys[(int)Binds::Back].GamepadButton]),
		});
		break;
	case DoomMenuItem::Type::Action:
		buttonGuide.SetButtons({
			fmt::format(u8"!{} Activate", GamepadPUAMap[Inputs.Keys[(int)Binds::Accept].GamepadButton]),
			fmt::format(u8"{} Back", GamepadPUAMap[Inputs.Keys[(int)Binds::Back].GamepadButton]),
		});
		break;
	case DoomMenuItem::Type::Options:
		buttonGuide.SetButtons({
			fmt::format(u8"!{} {} Advance", GamepadPUAMap[Inputs.Keys[(int)Binds::Accept].GamepadButton], GamepadPUAMap[Inputs.Keys[(int)Binds::Right].GamepadButton]),
			fmt::format(u8"{} Previous", GamepadPUAMap[Inputs.Keys[(int)Binds::Left].GamepadButton]),
			fmt::format(u8"{} Back", GamepadPUAMap[Inputs.Keys[(int)Binds::Back].GamepadButton]),
		});
		break;
	case DoomMenuItem::Type::Slider:
		buttonGuide.SetButtons({
			fmt::format(u8"{} {} Change", GamepadPUAMap[Inputs.Keys[(int)Binds::Left].GamepadButton], GamepadPUAMap[Inputs.Keys[(int)Binds::Right].GamepadButton]),
			fmt::format(u8"{} Back", GamepadPUAMap[Inputs.Keys[(int)Binds::Back].GamepadButton]),
		});
		break;
	case DoomMenuItem::Type::Checkbox:
		buttonGuide.SetButtons({
			fmt::format(u8"!{} Toggle", GamepadPUAMap[Inputs.Keys[(int)Binds::Accept].GamepadButton]),
			fmt::format(u8"{} Back", GamepadPUAMap[Inputs.Keys[(int)Binds::Back].GamepadButton]),
		});
		break;
	case DoomMenuItem::Type::Page:
		buttonGuide.SetButtons({
			fmt::format(u8"!{} Go", GamepadPUAMap[Inputs.Keys[(int)Binds::Accept].GamepadButton]),
			fmt::format(u8"{} Back", GamepadPUAMap[Inputs.Keys[(int)Binds::Back].GamepadButton]),
		});
		break;
	default:
		buttonGuide.SetButtons({
			fmt::format(u8"{} Back", GamepadPUAMap[(int)Binds::Back]),
		});
	}
}

DoomMenu::DoomMenu()
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

void DoomMenu::Tick(float dt)
{
	dt;

	static bool justSwitchedPage = true;

	if (remapping != -1)
	{
		if (Inputs.HaveGamePad)
		{
			GLFWgamepadstate state;
			if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state))
			{
				if (remapBounce)
				{
					int k = 0;
					for (int i = 0; i < 15; i++)
						k += state.buttons[i];
					remapBounce = (k > 0);
				}
				else
				{
					for (int i = 0; i < 15; i++)
					{
						if (state.buttons[i])
						{
							auto item = items->items[remapping];
							Inputs.Keys[item->selection].GamepadButton = i;
							//Inputs.Clear((Binds)item->selection);
							Inputs.Clear();
							remapping = -1;
							break;
						}
					}
				}
			}
		}
		return; //do not listen while assigning a key.
	}

	if (itemY.size() > 0 && Inputs.MouseMoved())
	{
		const int col = (int)(400 * scale);
		mouseHighlight = -1;
		if (Inputs.MousePosition.x >= itemX && Inputs.MousePosition.x <= itemX + (col * 2.5f))
		{
			for (int i = 0; i < itemY.size() - 1; i++)
			{
				if (Inputs.MousePosition.y >= itemY[i] && Inputs.MousePosition.y < itemY[i + 1])
				{
					mouseHighlight = highlight = i + scroll;
					break;
				}
			}
		}
	}
	cursor->Select(0);
	if (mouseHighlight != -1 && items->items[highlight]->type == DoomMenuItem::Type::Slider)
	{
		if (Inputs.MousePosition.x >= sliderStart && Inputs.MousePosition.x <= sliderEnd)
		{
			if (sliderHolding == -1)
				sliderHolding = highlight;
			if (highlight == sliderHolding)
			{
				cursor->Select(5);
				if (Inputs.MouseHoldLeft)
				{
					cursor->Select(6);
					auto item = items->items[highlight];

					//thanks GZDoom
					auto x = clamp(Inputs.MousePosition.x, sliderStart, sliderEnd);
					auto  v = item->minVal + ((x - sliderStart) * (item->maxVal - item->minVal)) / (sliderEnd - sliderStart);
					item->selection = (int)(round(v / item->step) * item->step);
					if (item->change != nullptr)
						item->change(item);
				}
				else
					sliderHolding = -1;
			}
		}
	}
	if (mouseHighlight != highlight && Inputs.MouseLeft)
		Inputs.MouseLeft = false;

	while (items->items[highlight]->type == DoomMenuItem::Type::Text)
		highlight++;

	if (justSwitchedPage)
	{
		if (Inputs.HaveGamePad)
		{
			GLFWgamepadstate state;
			if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state))
			{
				int k = 0;
				for (int i = 0; i < 15; i++)
					k += state.buttons[i];
				if (k)
					return;
			}
		}
		justSwitchedPage = false;
		Inputs.Clear(true);
		return;
	}

	if (Inputs.KeyDown(Binds::Up))
	{
		Inputs.Clear();
		if (highlight == 0)
		{
			highlight = (int)items->items.size();
			scroll = (int)items->items.size() - visible;
			if (scroll < 0) scroll = 0;
		}
		highlight--;
		if (highlight <= scroll)
			scroll--;
		if (scroll == -1)
		{
			scroll = 0;
			highlight = 0;
		}
		UpdateButtonGuide();
	}
	else if (Inputs.KeyDown(Binds::Down))
	{
		Inputs.Clear();
		highlight++;
		if (highlight - scroll >= visible)
			scroll++;
		if (highlight == items->items.size())
		{
			highlight = 0;
			scroll = 0;
		}
		UpdateButtonGuide();
	}

	if (highlight == -1)
		return;

	auto item = items->items[highlight];

	if (item->type == DoomMenuItem::Type::Page)
	{
		if (Inputs.KeyDown(Binds::Accept) || Inputs.MouseLeft)
		{
			stack.push(item->page);
			items = item->page;
			highlight = 0;
			scroll = 0;
			mouseHighlight = -1;
			Inputs.Clear(true);
			justSwitchedPage = true;
			UpdateButtonGuide();
		}
	}
	else if (item->type == DoomMenuItem::Type::Action)
	{
		if (Inputs.KeyDown(Binds::Accept) || Inputs.MouseLeft)
		{
			Inputs.Clear(Binds::Accept);
			item->Beep();
			if (item->change != nullptr)
				item->change(item);
		}
	}
	else if (item->type == DoomMenuItem::Type::KeyBind)
	{
		if (Inputs.KeyDown(Binds::Accept) || Inputs.MouseLeft)
		{
			Inputs.Clear(Binds::Accept);
			remapping = highlight;
			remapBounce = true;
		}
	}
	else if (item->type == DoomMenuItem::Type::Checkbox)
	{
		if (Inputs.KeyDown(Binds::Accept) || Inputs.MouseLeft)
		{
			item->selection ^= 1;
			item->Beep();
			if (item->change != nullptr)
				item->change(item);
		}
	}
	else if (item->type == DoomMenuItem::Type::Options)
	{
		if (Inputs.KeyDown(Binds::Accept) || Inputs.MouseLeft)
		{
			Inputs.Clear(Binds::Accept);
			Inputs.Keys[(int)Binds::Right].State = true;
			Inputs.MouseLeft = false;
		}
		if (Inputs.KeyDown(Binds::Left))
		{
			Inputs.Clear();
			if (item->selection == 0) item->selection = (int)item->options.size();
			item->selection--;
			if (item->change != nullptr)
				item->change(item);
		}
		else if (Inputs.KeyDown(Binds::Right))
		{
			Inputs.Clear();
			item->selection++;
			if (item->selection == item->options.size()) item->selection = 0;
			if (item->change != nullptr)
				item->change(item);
		}
	}
	else if (item->type == DoomMenuItem::Type::Slider)
	{
		if (Inputs.KeyDown(Binds::Left))
		{
			Inputs.Clear();
			if (item->selection > item->minVal)
			{
				item->selection -= item->step;
				item->Beep();
				if (item->change != nullptr)
					item->change(item);
			}
		}
		else if (Inputs.KeyDown(Binds::Right))
		{
			Inputs.Clear();
			if (item->selection < item->maxVal)
			{
				item->selection += item->step;
				item->Beep();
				if (item->change != nullptr)
					item->change(item);
			}
		}
	}

	if (Inputs.KeyDown(Binds::Back))
	{
		Inputs.Clear(Binds::Back);
		if (stack.size() > 1)
		{
			highlight = 0;
			scroll = 0;
			mouseHighlight = -1;
			stack.pop();
			items = stack.top();
			justSwitchedPage = true;
			UpdateButtonGuide();
			return;
		}
	}

	Inputs.Clear(Binds::Accept);
	Inputs.Clear(Binds::Back);
	Inputs.MouseLeft = false;
}

void DoomMenu::Draw(float dt)
{
	dt;
	const int col = (int)(400 * scale);

	const float startX = (width * 0.5f) - ((col * 3) * 0.5f);
	float startY = 56 * scale;
	float endY = height - (176 * scale);
	
	auto pos = glm::vec2(startX, startY);
	
	auto& controls = *UI::controls;

	itemX = pos.x;

	if (!items->header.empty())
	{
		auto headerW = sprender.MeasureText(1, items->header, 150).x;
		auto headerX = (width / 2) - (headerW / 2);

		sprender.DrawSprite(panels, glm::vec2(headerX - panels[4].z, pos.y) * scale, glm::vec2(panels[4].z, panels[4].w) * scale, panels[4], 0.0f, UI::themeColors["primary"]);
		sprender.DrawSprite(panels, glm::vec2(headerX, pos.y) * scale, glm::vec2(headerW, panels[3].w) * scale, panels[3], 0.0f, UI::themeColors["primary"]);
		sprender.DrawSprite(panels, glm::vec2(headerX + headerW, pos.y) * scale, glm::vec2(panels[5].z, panels[5].w) * scale, panels[5], 0.0f, UI::themeColors["primary"]);

		sprender.DrawText(1, items->header, glm::vec2(headerX, pos.y + 32), glm::vec4(1), 150);
		pos.y += panels[4].w + 32;

		if (!items->subheader.empty())
		{
			auto xy = sprender.MeasureText(1, items->subheader, 120);
			headerX = (width / 2) - (xy.x / 2);

			sprender.DrawSprite(*whiteRect, glm::vec2(0, pos.y) * scale, glm::vec2(width, xy.y + 16) * scale, glm::vec4(0), 0.0f, UI::themeColors["primary"]);

			sprender.DrawText(1, items->subheader, glm::vec2(headerX, pos.y + 8), glm::vec4(1), 120);
			pos.y += xy.y + 20;
		}

		startY = pos.y + 24;
	}

	const auto shown = std::min(visible, (int)items->items.size() - scroll);

	const auto partSize = controls[4].w * 0.75f *  scale;
	const auto thumbSize = glm::vec2(controls[3].z, controls[3].w) * 0.75f * scale;

	itemY.clear();

	sprender.DrawSprite(*whiteRect, glm::vec2(0, startY - 8) * scale, glm::vec2(width, endY - startY - 8) * scale, glm::vec4(0), 0.0f, UI::themeColors["primary"]);
	sprender.DrawSprite(*whiteRect, glm::vec2(0, endY) * scale, glm::vec2(width, 24) * scale, glm::vec4(0), 0.0f, UI::themeColors["primary"]);

	for (int i = 0; i < shown; i++)
	{
		auto item = items->items[i + scroll];
		auto size = 100 * scale;
		pos.y += (40 * scale) + size - (100 * scale);
		if (i + scroll == highlight)
		{
			auto offset = glm::vec2(item->type == DoomMenuItem::Type::Checkbox ? (40 * scale) : 0, 0);
			auto highlightSize = sprender.MeasureText(1, item->caption, 100 * scale);
			highlightSize.x += 8 * scale;
			highlightSize.y *= 0.75f;
			sprender.DrawSprite(controls, pos + offset + glm::vec2(-(highlightSize.y) * scale, 0), glm::vec2(highlightSize.y), controls[7], 0, UI::themeColors["secondary"]);
			sprender.DrawSprite(controls, pos + offset + glm::vec2(highlightSize.x, 0), glm::vec2(highlightSize.y), controls[8], 0, UI::themeColors["secondary"]);
			sprender.DrawSprite(controls, pos + offset, highlightSize, controls[9], 0, UI::themeColors["secondary"]);
			break;
		}
	}
	pos.y = startY;

	for (int i = 0; i < shown; i++)
	{
		auto item = items->items[i + scroll];
		auto color = UI::themeColors["white"];
		auto offset = glm::vec2(item->type == DoomMenuItem::Type::Checkbox ? (40 * scale) : 0, 0);
		auto font = 1;
		auto size = 100 * scale;

		if (item->type == DoomMenuItem::Type::Text)
		{
			font = item->selection;
			size = item->maxVal * scale;
		}
		else if (item->type == DoomMenuItem::Type::KeyBind && i + scroll == remapping)
		{
			color = UI::themeColors["yellow"];
		}

		sprender.DrawText(font, item->caption, pos + offset, color, size);

		if (item->type == DoomMenuItem::Type::Options)
		{
			sprender.DrawText(1, item->options[item->selection], pos + glm::vec2(col, 0), color, size);
		}
		else if (item->type == DoomMenuItem::Type::Slider)
		{
			if (item->format != nullptr)
			{
				auto fmt = item->format(item);
				sprender.DrawText(1, fmt, pos + glm::vec2(col + col + (94 * scale), 10), color, size * 0.75f);
			}
		}
		else if (item->type == DoomMenuItem::Type::KeyBind)
		{
			auto key = Inputs.Keys[item->selection];
			sprender.DrawText(1,key.Name, pos + glm::vec2(col, 0), color, size);
			
			sprender.DrawText(1, key.GamepadButton == -1 ? "[none]" : GamepadPUAMap[key.GamepadButton], pos + glm::vec2(col * 2, 0), color, size);
		}

		itemY.push_back(pos.y);
		pos.y += (40 * scale) + size - (100 * scale);
	}

	//terminator
	itemY.push_back(pos.y);

	if (items == &species)
	{
		sprender.DrawText(1, speciesText, glm::vec2(width * 0.6f, height * 0.4f), glm::vec4(1), 75.0f);
	}

	for (int i = 0; i < shown; i++)
	{
		auto item = i == 0 ? items->items[0] : items->items[i + scroll];
		const auto offset = glm::vec2(item->type == DoomMenuItem::Type::Checkbox ? (40 * scale) : 0, 0);

		pos.y = itemY[i];

		if (item->type == DoomMenuItem::Type::Checkbox)
		{
			sprender.DrawSprite(controls, pos + glm::vec2(0, 4 * scale), glm::vec2(partSize), controls[4], 0, UI::themeColors["secondary"]);
			if (item->selection)
				sprender.DrawSprite(controls, pos + glm::vec2(0, 4 * scale), glm::vec2(partSize), controls[5], 0, UI::themeColors["white"]);
		}
		else if (item->type == DoomMenuItem::Type::Slider)
		{
			const auto color = UI::themeColors["white"];
			auto barLength = col;
			sprender.DrawSprite(controls, pos + glm::vec2(col, 10 * scale), glm::vec2(partSize), controls[0], 0, color);
			sprender.DrawSprite(controls, pos + glm::vec2(col + barLength + (partSize * 1), 10 * scale), glm::vec2(partSize), controls[1], 0, color);
			sprender.DrawSprite(controls, pos + glm::vec2(col + partSize, 10 * scale), glm::vec2(barLength, partSize), controls[2], 0, color);

			sliderStart = pos.x + col + partSize;
			sliderEnd = sliderStart + barLength;

			//thanks GZDoom
			auto range = item->maxVal - item->minVal;
			auto ccur = clamp(item->selection, item->minVal, item->maxVal) - item->minVal;
			auto thumbPos = partSize + ((ccur * (barLength - (partSize * 2))) / range);

			auto thumb = glm::vec2(col + (int)thumbPos, 10 * scale);
			sprender.DrawSprite(controls, pos + thumb, thumbSize, controls[3], 0, color);
		}
	}

	//species page special stuff
	if (items == &species && items->items[highlight]->type == DoomMenuItem::Type::Checkbox)
	{
		sprender.DrawSprite(*speciesPreviews[highlight], glm::vec2((width * 0.5f) - (speciesPreviews[0]->width * 0.5f), (height * 0.5f) - (speciesPreviews[0]->height * 0.5f)));
	}

	buttonGuide.Draw();
}

bool DoomMenu::Scancode(unsigned int scancode)
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
	return true;
}
