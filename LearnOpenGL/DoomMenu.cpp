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

		options.clear();
		content.clear();
		volume.clear();

		options.push_back(new DoomMenuItem("Options", 2, 120));

		options.push_back(new DoomMenuItem("Content Manager...", &content));
		options.push_back(new DoomMenuItem("Language", 0, { "US English", u8"Japanese / 日本語", "German / Deutsch", "Spanish / español", u8"French / français", "Italian / italiano", "Hungarian / magyar", "Dutch / Nederlands" }, [&](DoomMenuItem*i) { dlgBox->Text(fmt::format("You chose <color:1>{}</color>.", i->options[i->selection])); }));
		options.push_back(new DoomMenuItem("Continue from", 0, { "Front door", "Main room", "Last used bed", "Last location" }));
		options.push_back(new DoomMenuItem("Speech", 1, { "Silence", "Bebebese", "Animalese" }));
		options.push_back(new DoomMenuItem("Ping rate", 2, 60, 3, 1, minutes));
		options.push_back(new DoomMenuItem("Balloon chance", 10, 60, 15, 5, percent));
		options.push_back(new DoomMenuItem("Cursor scale", 50, 150, 100, 10, percent, [&](DoomMenuItem*i) { cursor->SetScale(i->selection); }));
		options.push_back(new DoomMenuItem("Volume...", &volume));

		content.push_back(new DoomMenuItem("Content Manager", 2, 120));
		content.push_back(new DoomMenuItem("Venomous bugs <size:50>(tarantulas, scorpions et al)", true));
		content.push_back(new DoomMenuItem("Sea bass", true, [&](DoomMenuItem*i)
		{
			dlgBox->Text(i->selection ? "Whatever you say..." : "Aye aye, Miss Mayor! We'll start\npouring anti-freeze in their\nspawning grounds right away!");
		}));
		content.push_back(new DoomMenuItem("Cranky villagers", true));
		content.push_back(new DoomMenuItem("Horse villagers", true));
		content.push_back(new DoomMenuItem("Easter", true));
		content.push_back(back);

		volume.push_back(new DoomMenuItem("Volume", 2, 120));
		volume.push_back(new DoomMenuItem("Music", 0, 100, 70, 10, percent));
		volume.push_back(new DoomMenuItem("Ambience", 0, 100, 70, 10, percent));
		volume.push_back(new DoomMenuItem("Sound effects", 0, 100, 70, 10, percent));
		volume.push_back(new DoomMenuItem("Speech", 0, 100, 70, 10, percent));
		volume.push_back(back);
	}

DoomMenu::DoomMenu()
	{
		controls = new Texture("ui/controls.png");
		GetAtlas(controlsAtlas, "ui/controls.json");

		rebuild();

		highlight = 0;
		mouseHighlight = 0;
		scroll = 0;

		//to be filled in at first draw
		sliderStart = 0;
		sliderEnd = 1;
		sliderHolding = -1;
		itemX = 0;

		stack.push(options);
		items = &stack.top();
	}

void DoomMenu::Tick(double dt)
{
	//visible = (int)(12.0f * scale);

	if (Inputs.MouseMoved())
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

	const float startX = (width / 2) - ((col * 3) / 2);
	const float startY = 80;

	auto pos = glm::vec2(startX, startY);

	itemX = pos.x;

	const auto black = glm::vec4(0, 0, 0, 0.5);

	const auto start = items->at(0)->type == DoomMenuTypes::Text ? 1 : 0;
	const auto shown = std::min(visible, (int)items->size() - scroll);

	const auto partSize = controlsAtlas[4].w * 0.75f *  scale;
	const auto thumbSize = glm::vec2(controlsAtlas[3].z, controlsAtlas[3].w) * 0.75f * scale;

	sprender->DrawText(0, fmt::format("DoomMenu: {}/{} {} {},{} - {},{}", highlight, mouseHighlight, Inputs.MouseHoldLeft, Inputs.MousePosition.x, Inputs.MousePosition.y, sliderStart, sliderEnd), glm::vec2(0, 16));

	itemY.clear();

	pos.y -= 12 * scale;
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
			sprender->DrawSprite(controls, pos + offset + glm::vec2(-(highlightSize.y) * scale, 0), glm::vec2(highlightSize.y), controlsAtlas[7], 0, UI::themeColors["secondary"]);
			sprender->DrawSprite(controls, pos + offset + glm::vec2(highlightSize.x, 0), glm::vec2(highlightSize.y), controlsAtlas[8], 0, UI::themeColors["secondary"]);
			sprender->DrawSprite(controls, pos + offset, highlightSize, controlsAtlas[9], 0, UI::themeColors["secondary"]);
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

		sprender->DrawText(font, item->caption, pos + offset + glm::vec2(2), black, size);
		sprender->DrawText(font, item->caption, pos + offset, color, size);

		if (item->type == DoomMenuTypes::Options)
		{
			sprender->DrawText(1, item->options[item->selection], pos + glm::vec2(col + 2, 2), black, size);
			sprender->DrawText(1, item->options[item->selection], pos + glm::vec2(col, 0), color, size);
		}
		else if (item->type == DoomMenuTypes::Slider)
		{
			if (item->format != nullptr)
			{
				auto fmt = item->format(item);
				sprender->DrawText(1, fmt, pos + glm::vec2(col + col + (94 * scale) + 2, 12), black, size * 0.75f);
				sprender->DrawText(1, fmt, pos + glm::vec2(col + col + (94 * scale), 10), color, size * 0.75f);
			}
		}

		itemY.push_back(pos.y);
		pos.y += (40 * scale) + size - (100 * scale);
	}

	//terminator
	itemY.push_back(pos.y);

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
			sprender->DrawSprite(controls, pos + glm::vec2(0, 4 * scale), glm::vec2(partSize), controlsAtlas[4], 0, checkColor);
			if (item->selection)
				sprender->DrawSprite(controls, pos + glm::vec2(0, 4 * scale), glm::vec2(partSize), controlsAtlas[5], 0, color);
		}
		else if (item->type == DoomMenuTypes::Slider)
		{
			auto trackColor = color * glm::vec4(1, 1, 1, 0.5);
			auto barLength = col;
			//auto partSize = controlsAtlas[0].w * 0.5f * scale;
			sprender->DrawSprite(controls, pos + glm::vec2(col, 10 * scale), glm::vec2(partSize), controlsAtlas[0], 0, trackColor);
			sprender->DrawSprite(controls, pos + glm::vec2(col + barLength + (partSize * 1), 10 * scale), glm::vec2(partSize), controlsAtlas[1], 0, trackColor);
			sprender->DrawSprite(controls, pos + glm::vec2(col + partSize, 10 * scale), glm::vec2(barLength, partSize), controlsAtlas[2], 0, trackColor);

			sliderStart = pos.x + col + partSize;
			sliderEnd = sliderStart + barLength;

			//thanks GZDoom
			auto range = item->maxVal - item->minVal;
			auto ccur = clamp(item->selection, item->minVal, item->maxVal) - item->minVal;
			auto thumbPos = partSize + ((ccur * (barLength - (partSize * 2))) / range);

			auto thumb = glm::vec2(col + (int)thumbPos, 10 * scale);
			sprender->DrawSprite(controls, pos + thumb, thumbSize, controlsAtlas[3], 0, color);
		}
	}
}
