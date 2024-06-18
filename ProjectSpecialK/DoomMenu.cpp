#include "DoomMenu.h"
#include "InputsMap.h"
#include "Cursor.h"
#include "DialogueBox.h"

DoomMenuItem::DoomMenuItem(const std::string& cap, int val, std::initializer_list<std::string> opts, std::function<void(DoomMenuItem*)> chg = nullptr) : caption(cap), type(DoomMenuTypes::Options), selection(val), change(chg), page(nullptr)
{
	for (auto i : opts)
		options.emplace_back(i);
}

void DoomMenu::rebuild()
{
	auto back = new DoomMenuItem("Back", nullptr);
	back->type = DoomMenuTypes::Back;

	auto minutes = [&](DoomMenuItem* i)
	{
		return fmt::format("{} minutes", i->selection);
	};
	auto percent = [&](DoomMenuItem* i)
	{
		return fmt::format("{}%", i->selection);
	};

	static const Language opt2lan[] = { Language::USen, Language::JPja, Language::EUde, Language::EUes, Language::EUfr, Language::EUit, Language::EUhu, Language::EUnl };
	static const int lan2opt[] = { 0, 3, 4, 1, 0, 0, 0,	2, 0, 3, 4, 5, 7, 0, 6 };

	options.items.clear();
	content.items.clear();
	volume.items.clear();

	options.header = TextGet("menu:options:head");

	options.items.push_back(new DoomMenuItem(TextGet("menu:options:content"), &content));
	options.items.push_back(new DoomMenuItem(TextGet("menu:options:language"), lan2opt[(int)gameLang],
		{
			TextGet("menu:options:language:en"),
			TextGet("menu:options:language:jp"),
			TextGet("menu:options:language:de"),
			TextGet("menu:options:language:es"),
			TextGet("menu:options:language:fr"),
			TextGet("menu:options:language:it"),
			TextGet("menu:options:language:hu"),
			TextGet("menu:options:language:nl"),
		},
		[&](DoomMenuItem*i)
		{
			gameLang = opt2lan[i->selection];
			UI::settings["language"] = new JSONValue(i->selection);
			rebuild();
			items = &options;
			dlgBox->Text(fmt::format("You chose <color:1>{}</color>.", i->options[i->selection]));
		}
	));
	options.items.push_back(new DoomMenuItem(TextGet("menu:options:continuefrom"), (int)UI::settings["continue"]->AsNumber(),
		{
			TextGet("menu:options:continuefrom:0"),
			TextGet("menu:options:continuefrom:1"),
			TextGet("menu:options:continuefrom:2"),
			TextGet("menu:options:continuefrom:3"),
	},
		[&](DoomMenuItem*i) { UI::settings["continue"] = new JSONValue(i->selection); }
	));
	options.items.push_back(new DoomMenuItem(TextGet("menu:options:speech"), (int)UI::settings["speech"]->AsNumber(),
		{
			TextGet("menu:options:speech:0"),
			TextGet("menu:options:speech:1"),
			TextGet("menu:options:speech:2"),
		},
		[&](DoomMenuItem*i) { UI::settings["speech"] = new JSONValue(i->selection); }
	));
	options.items.push_back(new DoomMenuItem(TextGet("menu:options:pingrate"), 2, 60, (int)UI::settings["pingRate"]->AsNumber(), 1, minutes,
		[&](DoomMenuItem*i) { UI::settings["pingRate"] = new JSONValue(i->selection); }
	));
	options.items.push_back(new DoomMenuItem(TextGet("menu:options:balloonchance"), 10, 60, (int)UI::settings["balloonChance"]->AsNumber(), 5, percent,
		[&](DoomMenuItem*i) { UI::settings["balloonChance"] = new JSONValue(i->selection); }
	));
	options.items.push_back(new DoomMenuItem(TextGet("menu:options:cursorscale"), 50, 150, (int)UI::settings["cursorScale"]->AsNumber(), 10, percent,
		[&](DoomMenuItem*i)
		{
			cursor->SetScale(i->selection);
			UI::settings["cursorScale"] = new JSONValue(i->selection);
		}
	));
	options.items.push_back(new DoomMenuItem(TextGet("menu:options:volume"), &volume));

	content.header = TextGet("menu:options:head:content");
	{
		species.items.clear();
		species.header = TextGet("menu:options:head:content");
		species.subheader = TextGet("menu:options:content:species");
		for (const auto& s : ::species)
		{
			auto f = "filter:species:" + s.ID;
			//TODO: handle species having custom filters.
			if (s.ID == "bul") continue;

			auto sn = StripMSBT(TextGet(s.RefName + ":m"));
			if (std::islower(sn[0]))
				sn[0] = std::toupper(sn[0]);

			TextAdd(f, sn);
			if (Database::Filters.find(f) == Database::Filters.end())
				Database::Filters[f] = true;

			species.items.push_back(new DoomMenuItem(TextGet((std::string&)f), Database::Filters[f],
				[&, f](DoomMenuItem*i)
			{
				Database::Filters[f] = i->selection > 0;
				auto s = UI::settings["contentFilters"]->AsObject(); //-V836 can't be helped for now
				s.insert_or_assign(f, new JSONValue(Database::Filters[f]));
				UI::settings["contentFilters"] = new JSONValue(s);
			}
			));
		}
		species.items.push_back(back);

		speciesText = TextGet("menu:options:content:species:help");

		content.items.push_back(new DoomMenuItem(TextGet("menu:options:content:species"), &species));
	}

	for (const auto& fc : Database::FilterCategories)
	{
		auto fck = fc.first;
		
		auto fcpage = new DoomMenuPage(TextGet("menu:options:head:content"), TextGet(fck));
		for (const auto& f : fc.second)
		{

			fcpage->items.push_back(new DoomMenuItem(TextGet((std::string&)f), Database::Filters[f],
				[&, f](DoomMenuItem*i)
			{
				Database::Filters[f] = i->selection > 0;
				auto s = UI::settings["contentFilters"]->AsObject(); //-V836 can't be helped for now
				s.insert_or_assign(f, new JSONValue(Database::Filters[f]));
				UI::settings["contentFilters"] = new JSONValue(s);
			}
			));
		}

		fcpage->items.push_back(back);
		//TODO: Description field

		content.items.push_back(new DoomMenuItem(TextGet(fck), fcpage));
	}
	content.items.push_back(back);

	volume.header = TextGet("menu:options:head:volume");
	volume.items.push_back(new DoomMenuItem(TextGet("menu:options:volume:music"), 0, 100, (int)(Audio::MusicVolume * 100), 10, percent,
		[&](DoomMenuItem*i) { Audio::MusicVolume = i->selection / 100.0f; }
	));
	volume.items.push_back(new DoomMenuItem(TextGet("menu:options:volume:ambience"), 0, 100, (int)(Audio::AmbientVolume * 100), 10, percent,
		[&](DoomMenuItem*i) { Audio::AmbientVolume = i->selection / 100.0f; }
	));
	volume.items.push_back(new DoomMenuItem(TextGet("menu:options:volume:sfx"), 0, 100, (int)(Audio::SoundVolume * 100), 10, percent,
		[&](DoomMenuItem*i) { Audio::SoundVolume = i->selection / 100.0f; }
	));
	volume.items.push_back(new DoomMenuItem(TextGet("menu:options:volume:speech"), 0, 100, (int)(Audio::SpeechVolume * 100), 10, percent,
		[&](DoomMenuItem*i) { Audio::SpeechVolume = i->selection / 100.0f; }
	));
	volume.items.push_back(back);
}

DoomMenu::DoomMenu()
{
	rebuild();

	highlight = 0;
	mouseHighlight = 0;
	scroll = 0;

	//to be filled in at first draw
	sliderStart = 0;
	sliderEnd = 1;
	sliderHolding = -1;
	itemX = 0;

	stack.push(&options);
	items = stack.top();

	for (const auto& s : ::species)
	{
		//TODO: properly handle species having custom filters.
		if (s.ID == "bul") continue;
		speciesPreviews.push_back(new Texture("ui/species/" + s.ID + ".png"));
	}
}

void DoomMenu::Tick(double dt)
{
	//visible = (int)(12.0f * scale);

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
					mouseHighlight = highlight = i;
					break;
				}
			}
		}
	}
	cursor->Select(0);
	if (mouseHighlight != -1 && items->items[highlight]->type == DoomMenuTypes::Slider)
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
					/*
					auto ccur = clamp(item->selection, item->minVal, item->maxVal) - item->minVal;
					auto thumbPos = partSize + ((ccur * (barLength - (partSize * 2))) / range);
					*/
					//return;
				}
				else
					sliderHolding = -1;
			}
		}
	}
	if (mouseHighlight != highlight && Inputs.MouseLeft)
		Inputs.MouseLeft = false;

	while (items->items[highlight]->type == DoomMenuTypes::Text)
		highlight++;

	if (Inputs.Escape)
	{
		Inputs.Escape = false;
		if (stack.size() > 1)
		{
			highlight = 0;
			mouseHighlight = -1;
			stack.pop();
			items = stack.top();
			return;
		}
	}

	if (Inputs.Up)
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
	}
	else if (Inputs.Down)
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
	}

	if (highlight == -1)
		return;

	auto item = items->items[highlight];

	if (item->type == DoomMenuTypes::Page)
	{
		if (Inputs.Enter || Inputs.MouseLeft)
		{
			stack.push(item->page);
			items = item->page;
			highlight = 0;
			mouseHighlight = -1;
		}
	}
	else if (item->type == DoomMenuTypes::Back)
	{
		if (Inputs.Enter || Inputs.MouseLeft)
		{
			if (stack.size() > 1)
			{
				highlight = 0;
				stack.pop();
				items = stack.top();
				Inputs.Clear();
				return;
			}
		}
	}
	else if (item->type == DoomMenuTypes::Checkbox)
	{
		if (Inputs.Enter || Inputs.MouseLeft)
		{
			item->selection ^= 1;
			if (item->change != nullptr)
				item->change(item);
		}
	}
	else if (item->type == DoomMenuTypes::Options)
	{
		if (Inputs.Enter || Inputs.MouseLeft)
		{
			Inputs.Enter = false;
			Inputs.Right = true;
			Inputs.MouseLeft = false;
		}
		if (Inputs.Left)
		{
			Inputs.Clear();
			if (item->selection == 0) item->selection = (int)item->options.size();
			item->selection--;
			if (item->change != nullptr)
				item->change(item);
		}
		else if (Inputs.Right)
		{
			Inputs.Clear();
			item->selection++;
			if (item->selection == item->options.size()) item->selection = 0;
			if (item->change != nullptr)
				item->change(item);
		}
	}
	else if (item->type == DoomMenuTypes::Slider)
	{
		if (Inputs.Left)
		{
			Inputs.Clear();
			if (item->selection > item->minVal)
			{
				item->selection -= item->step;
				if (item->change != nullptr)
					item->change(item);
			}
		}
		else if (Inputs.Right)
		{
			Inputs.Clear();
			if (item->selection < item->maxVal)
			{
				item->selection += item->step;
				if (item->change != nullptr)
					item->change(item);
			}
		}
	}

	Inputs.Enter = false;
	Inputs.MouseLeft = false;
}

void DoomMenu::Draw(double dt)
{
	const int col = (int)(400 * scale);

	const float startX = (width * 0.5f) - ((col * 3) * 0.5f);
	float startY = 56 * scale;
	float endY = height - (176 * scale);
	
	auto pos = glm::vec2(startX, startY);
	
	auto& controls = *UI::controls;

	itemX = pos.x;

	if (!items->header.empty())
	{
		auto headerW = sprender->MeasureText(1, items->header, 150).x;
		auto headerX = (width / 2) - (headerW / 2);

		sprender->DrawSprite(panels, glm::vec2(headerX - panels[4].z, pos.y) * scale, glm::vec2(panels[4].z, panels[4].w) * scale, panels[4], 0.0f, UI::themeColors["primary"]);
		sprender->DrawSprite(panels, glm::vec2(headerX, pos.y) * scale, glm::vec2(headerW, panels[3].w) * scale, panels[3], 0.0f, UI::themeColors["primary"]);
		sprender->DrawSprite(panels, glm::vec2(headerX + headerW, pos.y) * scale, glm::vec2(panels[5].z, panels[5].w) * scale, panels[5], 0.0f, UI::themeColors["primary"]);

		sprender->DrawText(1, items->header, glm::vec2(headerX, pos.y + 32), glm::vec4(1), 150);
		pos.y += panels[4].w + 32;

		if (!items->subheader.empty())
		{
			auto xy = sprender->MeasureText(1, items->subheader, 120);
			headerX = (width / 2) - (xy.x / 2);

			sprender->DrawSprite(*whiteRect, glm::vec2(0, pos.y) * scale, glm::vec2(width, xy.y + 16) * scale, glm::vec4(0), 0.0f, UI::themeColors["primary"]);

			sprender->DrawText(1, items->subheader, glm::vec2(headerX, pos.y + 8), glm::vec4(1), 120);
			pos.y += xy.y + 20;
		}

		startY = pos.y + 24;
	}

	const auto shown = std::min(visible, (int)items->items.size() - scroll);

	const auto partSize = controls[4].w * 0.75f *  scale;
	const auto thumbSize = glm::vec2(controls[3].z, controls[3].w) * 0.75f * scale;

	//sprender->DrawText(0, fmt::format("DoomMenu: {}/{} {} {},{} - {},{}", highlight, mouseHighlight, Inputs.MouseHoldLeft, Inputs.MousePosition.x, Inputs.MousePosition.y, sliderStart, sliderEnd), glm::vec2(0, 16));

	itemY.clear();

	sprender->DrawSprite(*whiteRect, glm::vec2(0, startY - 8) * scale, glm::vec2(width, endY - startY - 8) * scale, glm::vec4(0), 0.0f, UI::themeColors["primary"]);
	sprender->DrawSprite(*whiteRect, glm::vec2(0, endY) * scale, glm::vec2(width, 24) * scale, glm::vec4(0), 0.0f, UI::themeColors["primary"]);

	//pos.y -= 8 * scale;
	//pos.y = startY + headerH;
	for (int i = 0; i < shown; i++)
	{
		auto item = items->items[i + scroll];
		auto size = 100 * scale;
		pos.y += (40 * scale) + size - (100 * scale);
		if (i + scroll == highlight)
		{
			auto offset = glm::vec2(item->type == DoomMenuTypes::Checkbox ? (40 * scale) : 0, 0);
			auto highlightSize = sprender->MeasureText(1, item->caption, 100 * scale);
			highlightSize.x += 8 * scale;
			highlightSize.y *= 0.75f;
			sprender->DrawSprite(controls, pos + offset + glm::vec2(-(highlightSize.y) * scale, 0), glm::vec2(highlightSize.y), controls[7], 0, UI::themeColors["secondary"]);
			sprender->DrawSprite(controls, pos + offset + glm::vec2(highlightSize.x, 0), glm::vec2(highlightSize.y), controls[8], 0, UI::themeColors["secondary"]);
			sprender->DrawSprite(controls, pos + offset, highlightSize, controls[9], 0, UI::themeColors["secondary"]);
			break;
		}
	}
	pos.y = startY;

	for (int i = 0; i < shown; i++)
	{
		auto item = items->items[i + scroll];
		//auto color = glm::vec4(1, 1, i + scroll == highlight ? 0.25 : 1, 1);
		const auto color = glm::vec4(1);
		auto offset = glm::vec2(item->type == DoomMenuTypes::Checkbox ? (40 * scale) : 0, 0);
		auto font = 1;
		auto size = 100 * scale;

		if (item->type == DoomMenuTypes::Text)
		{
			font = item->selection;
			size = item->maxVal * scale;
		}

		//sprender->DrawText(font, item->caption, pos + offset + glm::vec2(2), black, size);
		sprender->DrawText(font, item->caption, pos + offset, color, size);

		if (item->type == DoomMenuTypes::Options)
		{
			//sprender->DrawText(1, item->options[item->selection], pos + glm::vec2(col + 2, 2), black, size);
			sprender->DrawText(1, item->options[item->selection], pos + glm::vec2(col, 0), color, size);
		}
		else if (item->type == DoomMenuTypes::Slider)
		{
			if (item->format != nullptr)
			{
				auto fmt = item->format(item);
				//sprender->DrawText(1, fmt, pos + glm::vec2(col + col + (94 * scale) + 2, 12), black, size * 0.75f);
				sprender->DrawText(1, fmt, pos + glm::vec2(col + col + (94 * scale), 10), color, size * 0.75f);
			}
		}

		itemY.push_back(pos.y);
		pos.y += (40 * scale) + size - (100 * scale);
	}

	//terminator
	itemY.push_back(pos.y);

	if (items == &species)
	{
		sprender->DrawText(1, speciesText, glm::vec2(width * 0.6f, height * 0.4f), glm::vec4(1), 75.0f);
	}

	for (int i = 0; i < shown; i++)
	{
		auto item = i == 0 ? items->items[0] : items->items[i + scroll];
		auto color = glm::vec4(1, 1, i + scroll == highlight ? 0.25 : 1, 1);
		auto offset = glm::vec2(item->type == DoomMenuTypes::Checkbox ? (40 * scale) : 0, 0);
		auto font = 1;
		auto size = 100 * scale;

		pos.y = itemY[i];

		if (item->type == DoomMenuTypes::Checkbox)
		{
			auto checkColor = color * glm::vec4(1, 1, 1, 0.5);
			sprender->DrawSprite(controls, pos + glm::vec2(0, 4 * scale), glm::vec2(partSize), controls[4], 0, checkColor);
			if (item->selection)
				sprender->DrawSprite(controls, pos + glm::vec2(0, 4 * scale), glm::vec2(partSize), controls[5], 0, color);
		}
		else if (item->type == DoomMenuTypes::Slider)
		{
			auto trackColor = color * glm::vec4(1, 1, 1, 0.5);
			auto barLength = col;
			//auto partSize = controlsAtlas[0].w * 0.5f * scale;
			sprender->DrawSprite(controls, pos + glm::vec2(col, 10 * scale), glm::vec2(partSize), controls[0], 0, trackColor);
			sprender->DrawSprite(controls, pos + glm::vec2(col + barLength + (partSize * 1), 10 * scale), glm::vec2(partSize), controls[1], 0, trackColor);
			sprender->DrawSprite(controls, pos + glm::vec2(col + partSize, 10 * scale), glm::vec2(barLength, partSize), controls[2], 0, trackColor);

			sliderStart = pos.x + col + partSize;
			sliderEnd = sliderStart + barLength;

			//thanks GZDoom
			auto range = item->maxVal - item->minVal;
			auto ccur = clamp(item->selection, item->minVal, item->maxVal) - item->minVal;
			auto thumbPos = partSize + ((ccur * (barLength - (partSize * 2))) / range);

			auto thumb = glm::vec2(col + (int)thumbPos, 10 * scale);
			sprender->DrawSprite(controls, pos + thumb, thumbSize, controls[3], 0, color);
		}
	}

	//species page special stuff
	if (items == &species && items->items[highlight]->type == DoomMenuTypes::Checkbox)
	{
		sprender->DrawSprite(*speciesPreviews[highlight], glm::vec2((width * 0.5f) - (speciesPreviews[0]->width * 0.5f), (height * 0.5f) - (speciesPreviews[0]->height * 0.5f)));
	}
}
