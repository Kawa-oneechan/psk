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

	options.clear();
	content.clear();
	volume.clear();

	options.push_back(new DoomMenuItem(TextGet("menu:options:head"), 2, 120));

	options.push_back(new DoomMenuItem(TextGet("menu:options:content"), &content));
	options.push_back(new DoomMenuItem(TextGet("menu:options:language"), lan2opt[(int)gameLang],
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
	options.push_back(new DoomMenuItem(TextGet("menu:options:continuefrom"), (int)UI::settings["continue"]->AsNumber(),
		{
			TextGet("menu:options:continuefrom:0"),
			TextGet("menu:options:continuefrom:1"),
			TextGet("menu:options:continuefrom:2"),
			TextGet("menu:options:continuefrom:3"),
	},
		[&](DoomMenuItem*i) { UI::settings["continue"] = new JSONValue(i->selection); }
	));
	options.push_back(new DoomMenuItem(TextGet("menu:options:speech"), (int)UI::settings["speech"]->AsNumber(),
		{
			TextGet("menu:options:speech:0"),
			TextGet("menu:options:speech:1"),
			TextGet("menu:options:speech:2"),
		},
		[&](DoomMenuItem*i) { UI::settings["speech"] = new JSONValue(i->selection); }
	));
	options.push_back(new DoomMenuItem(TextGet("menu:options:pingrate"), 2, 60, (int)UI::settings["pingRate"]->AsNumber(), 1, minutes,
		[&](DoomMenuItem*i) { UI::settings["pingRate"] = new JSONValue(i->selection); }
	));
	options.push_back(new DoomMenuItem(TextGet("menu:options:balloonchance"), 10, 60, (int)UI::settings["balloonChance"]->AsNumber(), 5, percent,
		[&](DoomMenuItem*i) { UI::settings["balloonChance"] = new JSONValue(i->selection); }
	));
	options.push_back(new DoomMenuItem(TextGet("menu:options:cursorscale"), 50, 150, (int)UI::settings["cursorScale"]->AsNumber(), 10, percent,
		[&](DoomMenuItem*i)
		{
			cursor->SetScale(i->selection);
			UI::settings["cursorScale"] = new JSONValue(i->selection);
		}
	));
	options.push_back(new DoomMenuItem(TextGet("menu:options:volume"), &volume));

	content.push_back(new DoomMenuItem(TextGet("menu:options:head:content"), 2, 120));
	{
		species.clear();
		species.push_back(new DoomMenuItem(TextGet("menu:options:head:content"), 2, 120));
		species.push_back(new DoomMenuItem(TextGet("menu:options:content:species"), 2, 100));
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

			species.push_back(new DoomMenuItem(TextGet((std::string&)f), Database::Filters[f],
				[&, f](DoomMenuItem*i)
			{
				Database::Filters[f] = i->selection > 0;
				auto s = UI::settings["contentFilters"]->AsObject(); //-V836 can't be helped for now
				s.insert_or_assign(f, new JSONValue(Database::Filters[f]));
				UI::settings["contentFilters"] = new JSONValue(s);
			}
			));
		}
		species.push_back(back);

		speciesText = TextGet("menu:options:content:species:help");

		content.push_back(new DoomMenuItem(TextGet("menu:options:content:species"), &species));
	}

	for (const auto& fc : Database::FilterCategories)
	{
		auto fck = fc.first;
		
		auto fcpage = new std::vector<DoomMenuItem*>();
		fcpage->push_back(new DoomMenuItem(TextGet("menu:options:head:content"), 2, 120));
		fcpage->push_back(new DoomMenuItem(TextGet(fck), 2, 100));
		for (const auto& f : fc.second)
		{

			fcpage->push_back(new DoomMenuItem(TextGet((std::string&)f), Database::Filters[f],
				[&, f](DoomMenuItem*i)
			{
				Database::Filters[f] = i->selection > 0;
				auto s = UI::settings["contentFilters"]->AsObject(); //-V836 can't be helped for now
				s.insert_or_assign(f, new JSONValue(Database::Filters[f]));
				UI::settings["contentFilters"] = new JSONValue(s);
			}
			));
		}

		fcpage->push_back(back);
		//TODO: Description field

		content.push_back(new DoomMenuItem(TextGet(fck), fcpage));
	}
	content.push_back(back);

	volume.push_back(new DoomMenuItem(TextGet("menu:options:head:volume"), 2, 120));
	volume.push_back(new DoomMenuItem(TextGet("menu:options:volume:music"), 0, 100, (int)(Audio::MusicVolume * 100), 10, percent,
		[&](DoomMenuItem*i) { Audio::MusicVolume = i->selection / 100.0f; }
	));
	volume.push_back(new DoomMenuItem(TextGet("menu:options:volume:ambience"), 0, 100, (int)(Audio::AmbientVolume * 100), 10, percent,
		[&](DoomMenuItem*i) { Audio::AmbientVolume = i->selection / 100.0f; }
	));
	volume.push_back(new DoomMenuItem(TextGet("menu:options:volume:sfx"), 0, 100, (int)(Audio::SoundVolume * 100), 10, percent,
		[&](DoomMenuItem*i) { Audio::SoundVolume = i->selection / 100.0f; }
	));
	volume.push_back(new DoomMenuItem(TextGet("menu:options:volume:speech"), 0, 100, (int)(Audio::SpeechVolume * 100), 10, percent,
		[&](DoomMenuItem*i) { Audio::SpeechVolume = i->selection / 100.0f; }
	));
	volume.push_back(back);
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

	panels = new Texture("ui/panels.png");
	GetAtlas(panelAtlas, "ui/panels.json");

	stack.push(options);
	items = &stack.top();

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
	if (mouseHighlight != -1 && items->at(highlight)->type == DoomMenuTypes::Slider)
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
					auto item = items->at(highlight);

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

	while (items->at(highlight)->type == DoomMenuTypes::Text)
		highlight++;

	if (Inputs.Escape)
	{
		Inputs.Escape = false;
		if (stack.size() > 1)
		{
			highlight = 0;
			mouseHighlight = -1;
			stack.pop();
			items = &stack.top();
			return;
		}
	}

	if (Inputs.Up)
	{
		Inputs.Clear();
		int top = items->at(0)->type == DoomMenuTypes::Text ? 1 : 0;
		top += items->at(1)->type == DoomMenuTypes::Text ? 1 : 0;
		if (highlight == top)
		{
			highlight = (int)items->size();
			scroll = (int)items->size() - visible;
			if (scroll < 0) scroll = 0;
		}
		highlight--;
		if (highlight <= scroll)
			scroll--;
		if (scroll == -1)
		{
			scroll = 0;
			highlight = top;
		}
		while (items->at(highlight - scroll)->type == DoomMenuTypes::Text)
		{
			if (highlight - scroll == top)
			{
				highlight = (int)items->size();
				scroll = (int)items->size() - visible;
			}
			highlight--;
		}
	}
	else if (Inputs.Down)
	{
		Inputs.Clear();
		highlight++;
		if (highlight - scroll >= visible)
			scroll++;
		if (highlight == items->size())
		{
			highlight = 0;
			scroll = 0;
		}
	}

	if (highlight == -1)
		return;

	auto item = items->at(highlight);

	if (item->type == DoomMenuTypes::Page)
	{
		if (Inputs.Enter || Inputs.MouseLeft)
		{
			stack.push(*item->page);
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
				items = &stack.top();
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
	const float startY = height * 0.15f;

	auto pos = glm::vec2(startX, startY);

	itemX = pos.x;

	const auto black = glm::vec4(0, 0, 0, 0.5);

	const auto start = items->at(0)->type == DoomMenuTypes::Text ? (items->at(1)->type == DoomMenuTypes::Text ? 2 : 1) : 0;
	const auto shown = std::min(visible, (int)items->size() - scroll);

	const auto partSize = UI::controlsAtlas[4].w * 0.75f *  scale;
	const auto thumbSize = glm::vec2(UI::controlsAtlas[3].z, UI::controlsAtlas[3].w) * 0.75f * scale;

	sprender->DrawText(0, fmt::format("DoomMenu: {}/{} {} {},{} - {},{}", highlight, mouseHighlight, Inputs.MouseHoldLeft, Inputs.MousePosition.x, Inputs.MousePosition.y, sliderStart, sliderEnd), glm::vec2(0, 16));

	itemY.clear();

	//Fucking redo this part. Have some means to Properly Know what a header is, for starters.
	/*
	auto headerX = 0.0f;
	auto headerH = 0.0f;

	auto headers = 0;

	//background
	if (items->at(0)->type == DoomMenuTypes::Text)
	{
		auto i = items->at(0);
		if (i->maxVal >= 150)
		{
			//it's the primary header
			headers++;
			auto wh = sprender->MeasureText(i->selection, i->caption, (float)i->maxVal);
			auto w = wh.x;
			headerH = startY + (panelAtlas[4].w * scale);
			headerX = (width / 2) - (w / 2);
			sprender->DrawSprite(panels, glm::vec2(headerX, startY - 32) * scale, glm::vec2(panelAtlas[4].z, panelAtlas[4].w) * scale, panelAtlas[4], 0.0f, UI::themeColors["primary"]);
			sprender->DrawSprite(panels, glm::vec2(headerX + panelAtlas[4].z, startY - 32) * scale, glm::vec2(w, panelAtlas[3].w) * scale, panelAtlas[3], 0.0f, UI::themeColors["primary"]);
			sprender->DrawSprite(panels, glm::vec2(headerX + panelAtlas[4].z + w, startY - 32) * scale, glm::vec2(panelAtlas[5].z, panelAtlas[5].w) * scale, panelAtlas[5], 0.0f, UI::themeColors["primary"]);
			//sprender->DrawSprite(panels, glm::vec2(0, startY), glm::vec2(width, (items->at(0)->maxVal + 16) * scale), panelAtlas[4]
		}
		if (items->at(1)->type == DoomMenuTypes::Text)
		{
			i = items->at(1);
			headers++;
			if (i->maxVal >= 120)
			{
				//secondary header
				auto h = sprender->MeasureText(i->selection, i->caption, (float)i->maxVal).y;
				sprender->DrawSprite(whiteRect, glm::vec2(0, headerH), glm::vec2(width, h) * scale, glm::vec4(0), 0.0f, UI::themeColors["primary"]);
				headerH += h * scale;
			}
		}
	}
	//sprender->DrawSprite(whiteRect, glm::vec2(0, startY + headerH), glm::vec2(width, (items->size() - headers) * 50) * scale, glm::vec4(0), 0.0f, UI::themeColors["primary"]);
	*/

	pos.y -= 12 * scale;
	//pos.y = startY + headerH;
	for (int i = 0; i < shown; i++)
	{
		auto item = i == 0 ? items->at(0) : items->at(i + scroll);
		auto size = 100 * scale;
		pos.y += (40 * scale) + size - (100 * scale);
		if (i + scroll == highlight) // (item->type == DoomMenuTypes::Text)
		{
			auto offset = glm::vec2(item->type == DoomMenuTypes::Checkbox ? (40 * scale) : 0, 0);
			auto highlightSize = sprender->MeasureText(1, item->caption, 100 * scale);
			highlightSize.x += 8 * scale;
			highlightSize.y *= 0.75f;
			//sprender->DrawSprite(whiteRect, pos + glm::vec2(-8 * scale, 4 * scale), highlightSize, controlsAtlas[4], 0, UI::themeColors["secondary"]);
			sprender->DrawSprite(UI::controls, pos + offset + glm::vec2(-(highlightSize.y) * scale, 0), glm::vec2(highlightSize.y), UI::controlsAtlas[7], 0, UI::themeColors["secondary"]);
			sprender->DrawSprite(UI::controls, pos + offset + glm::vec2(highlightSize.x, 0), glm::vec2(highlightSize.y), UI::controlsAtlas[8], 0, UI::themeColors["secondary"]);
			sprender->DrawSprite(UI::controls, pos + offset, highlightSize, UI::controlsAtlas[9], 0, UI::themeColors["secondary"]);
			break;
		}
	}
	pos.y = startY;

	for (int i = 0; i < shown; i++)
	{
		auto item = i == 0 ? items->at(0) : items->at(i + scroll);
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
		sprender->DrawText(1, speciesText, glm::vec2(width * 0.6f, height * 0.3f), glm::vec4(1), 75.0f);
	}

	for (int i = 0; i < shown; i++)
	{
		auto item = i == 0 ? items->at(0) : items->at(i + scroll);
		auto color = glm::vec4(1, 1, i + scroll == highlight ? 0.25 : 1, 1);
		auto offset = glm::vec2(item->type == DoomMenuTypes::Checkbox ? (40 * scale) : 0, 0);
		auto font = 1;
		auto size = 100 * scale;

		pos.y = itemY[i];

		if (item->type == DoomMenuTypes::Checkbox)
		{
			auto checkColor = color * glm::vec4(1, 1, 1, 0.5);
			sprender->DrawSprite(UI::controls, pos + glm::vec2(0, 4 * scale), glm::vec2(partSize), UI::controlsAtlas[4], 0, checkColor);
			if (item->selection)
				sprender->DrawSprite(UI::controls, pos + glm::vec2(0, 4 * scale), glm::vec2(partSize), UI::controlsAtlas[5], 0, color);
		}
		else if (item->type == DoomMenuTypes::Slider)
		{
			auto trackColor = color * glm::vec4(1, 1, 1, 0.5);
			auto barLength = col;
			//auto partSize = controlsAtlas[0].w * 0.5f * scale;
			sprender->DrawSprite(UI::controls, pos + glm::vec2(col, 10 * scale), glm::vec2(partSize), UI::controlsAtlas[0], 0, trackColor);
			sprender->DrawSprite(UI::controls, pos + glm::vec2(col + barLength + (partSize * 1), 10 * scale), glm::vec2(partSize), UI::controlsAtlas[1], 0, trackColor);
			sprender->DrawSprite(UI::controls, pos + glm::vec2(col + partSize, 10 * scale), glm::vec2(barLength, partSize), UI::controlsAtlas[2], 0, trackColor);

			sliderStart = pos.x + col + partSize;
			sliderEnd = sliderStart + barLength;

			//thanks GZDoom
			auto range = item->maxVal - item->minVal;
			auto ccur = clamp(item->selection, item->minVal, item->maxVal) - item->minVal;
			auto thumbPos = partSize + ((ccur * (barLength - (partSize * 2))) / range);

			auto thumb = glm::vec2(col + (int)thumbPos, 10 * scale);
			sprender->DrawSprite(UI::controls, pos + thumb, thumbSize, UI::controlsAtlas[3], 0, color);
		}
	}

	//species page special stuff
	if (items == &species && items->at(highlight)->type == DoomMenuTypes::Checkbox)
	{
		sprender->DrawSprite(speciesPreviews[highlight - start], glm::vec2((width * 0.5f) - (speciesPreviews[0]->width * 0.5f), (height * 0.4f) - (speciesPreviews[0]->height * 0.5f)));
	}
}
