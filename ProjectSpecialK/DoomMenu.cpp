#define GLFW_INCLUDE_NONE
#include <glfw/glfw3.h>
#include <format.h>
#include "engine/Cursor.h"
#include "engine/Text.h"
#include "engine/TextUtils.h"
#include "engine/Audio.h"
#include "engine/Utilities.h"
#include "engine/SpriteRenderer.h"
#include "DoomMenu.h"
#include "Types.h"
#include "Game.h"


//TODO: Add a thing where changing the game language reveals a warning about having to restart.
//Would also work for a few other "deep" settings like resolution or whatever.

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
		for (const auto& i : optionKeys)
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

		auto ccur = glm::clamp(selection, minVal, maxVal) - minVal;
		auto panpot = -1.0f + ((ccur / (float)(maxVal - minVal)) * 2.0f);

		beep->SetPan(panpot);
		beep->Play(true, false);
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

__declspec(noreturn)
extern void FatalError(const std::string& message);
void DoomMenu::Translate()
{
	FatalError("Tried to call DoomMenu::Translate().");
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
}

bool DoomMenu::Tick(float dt)
{
	dt;

	static bool justSwitchedPage = true;

	auto metrics = UI::json["metrics"].as_object();
	const int col = (int)(metrics["menuColumnSize"].as_number() * scale);

	const auto shown = std::min(items->subheader.empty() ? visible : visible - 3, (int)items->items.size() - scroll);

	if (remapping != -1)
	{
		if (Inputs.HaveGamePad)
		{
			GLFWgamepadstate state;
			if (glfwGetGamepadState(0, &state))
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
							const auto* item = items->items[remapping];
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
		return false; //do not listen while assigning a key.
	}

	if (itemY.size() > 0 && Inputs.MouseMoved())
	{
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
					auto x = glm::clamp(Inputs.MousePosition.x, sliderStart, sliderEnd);
					auto  v = item->minVal + ((x - sliderStart) * (item->maxVal - item->minVal)) / (sliderEnd - sliderStart);
					item->selection = glm::clamp((int)(round(v / item->step) * item->step), item->minVal, item->maxVal);
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
			if (glfwGetGamepadState(0, &state))
			{
				int k = 0;
				for (int i = 0; i < 15; i++)
					k += state.buttons[i];
				if (k)
					return false;
			}
		}
		justSwitchedPage = false;
		Inputs.Clear(true);
		return false;
	}

	if (Inputs.KeyDown(Binds::Up))
	{
		Inputs.Clear();
		if (highlight == 0)
		{
			highlight = (int)items->items.size();
			scroll = (int)items->items.size() - shown;
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
		if (highlight - scroll >= shown)
			scroll++;
		if (highlight == items->items.size())
		{
			highlight = 0;
			scroll = 0;
		}
		UpdateButtonGuide();
	}

	if (highlight == -1)
		return true;

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
				item->selection = glm::clamp(item->selection, item->minVal, item->maxVal);
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
				item->selection = glm::clamp(item->selection, item->minVal, item->maxVal);
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
			return false;
		}
		else
		{
			Dead = true;
		}
	}

	//returning false should clear all inputs
	return false;
}

static void gradientPanel(Texture& texture, float startY, float height, float col)
{
	auto width = 1980.0f;
	Sprite::DrawSprite(texture, glm::vec2(0, startY - 8) * scale, glm::vec2(col, height) * scale, texture[6], 0.0f, UI::themeColors["primary"]);
	Sprite::DrawSprite(texture, glm::vec2(width - col, startY - 8) * scale, glm::vec2(col, height) * scale, texture[7], 0.0f, UI::themeColors["primary"]);
	Sprite::DrawSprite(texture, glm::vec2(col, startY - 8) * scale, glm::vec2(width - col - col, height) * scale, texture[8], 0.0f, UI::themeColors["primary"]);
}

void DoomMenu::Draw(float dt)
{
	dt;
	auto width = 1980.0f;
	auto height = 1080.0f;

	auto metrics = UI::json["metrics"].as_object();
	const int col = (int)(metrics["menuColumnSize"].as_number() * scale);

	const float startX = (width * metrics["menuStartX"].as_number()) * scale;
	float startY = metrics["menuStartY"].as_number() * scale;
	const float endY = (height - metrics["menuEndDist"].as_number()) * scale;
	const float headerSize = metrics["menuHeaderSize"].as_number();
	const float headerOffset = metrics["menuHeaderOffset"].as_number();
	const float headerPadding = metrics["menuHeaderPadding"].as_number();
	const float subHeaderSize = metrics["menuSubHeaderSize"].as_number();
	const float subHeaderOffset = metrics["menuSubHeaderOffset"].as_number();
	const float subHeaderPadding = metrics["menuSubHeaderPadding"].as_number();
	const float itemSize = metrics["menuItemSize"].as_number();
	const float itemSpace = metrics["menuItemSpacing"].as_number();
	const float partScale = metrics["menuItemPartScale"].as_number();
	const float hiliteBarOffset = metrics["menuHiliteBarOffset"].as_number();
	const float checkboxOffset = metrics["menuCheckboxOffset"].as_number();
	const float sliderValueOffsetX = metrics["menuSliderValueOffsetX"].as_number();
	const float sliderValueOffsetY = metrics["menuSliderValueOffsetY"].as_number();

	auto pos = glm::vec2(startX, startY);
	
	auto& controls = *UI::controls;

	itemX = pos.x;

	if (!items->header.empty())
	{
		auto headerW = Sprite::MeasureText(1, items->header, headerSize).x;
		auto headerX = (width - headerW) / 2;

		Sprite::DrawSprite(panels, glm::vec2(headerX - panels[4].z, pos.y) * scale, glm::vec2(panels[4].z, panels[4].w) * scale, panels[4], 0.0f, UI::themeColors["primary"]);
		Sprite::DrawSprite(panels, glm::vec2(headerX, pos.y) * scale, glm::vec2(headerW, panels[3].w) * scale, panels[3], 0.0f, UI::themeColors["primary"]);
		Sprite::DrawSprite(panels, glm::vec2(headerX + headerW, pos.y) * scale, glm::vec2(panels[5].z, panels[5].w) * scale, panels[5], 0.0f, UI::themeColors["primary"]);

		Sprite::DrawText(1, items->header, glm::vec2(headerX, pos.y + headerOffset) * scale, glm::vec4(1), headerSize * scale);
		pos.y += panels[4].w + (32 * scale);

		if (!items->subheader.empty())
		{
			auto xy = Sprite::MeasureText(1, items->subheader, subHeaderSize);
			headerX = (width - xy.x) / 2;

			//Sprite::DrawSprite(*whiteRect, glm::vec2(0, pos.y) * scale, glm::vec2(width, xy.y + 16) * scale, glm::vec4(0), 0.0f, UI::themeColors["primary"]);
			gradientPanel(panels, pos.y, xy.y + 0, (float)col);

			Sprite::DrawText(1, items->subheader, glm::vec2(headerX, pos.y + subHeaderOffset) * scale, glm::vec4(1), subHeaderSize * scale);
			pos.y += xy.y + (subHeaderPadding * scale);
		}

		startY = pos.y + (headerPadding * scale);
	}

	const auto shown = std::min(items->subheader.empty() ? visible : visible - 3, (int)items->items.size() - scroll);

	const auto partSize = controls[4].w * partScale * scale;
	const auto thumbSize = glm::vec2(controls[3].z, controls[3].w) * partScale * scale;

	itemY.clear();

	//Sprite::DrawSprite(*whiteRect, glm::vec2(0, startY - 8) * scale, glm::vec2(width, endY - startY - 8) * scale, glm::vec4(0), 0.0f, UI::themeColors["primary"]);
	//Sprite::DrawSprite(*whiteRect, glm::vec2(0, endY) * scale, glm::vec2(width, 24) * scale, glm::vec4(0), 0.0f, UI::themeColors["primary"]);
	gradientPanel(panels, startY - 8, endY - startY - 8, (float)col);
	gradientPanel(panels, endY, 24, (float)col);
	/*
	Sprite::DrawSprite(panels, glm::vec2(0, startY - 8) * scale, glm::vec2(col, endY - startY - 8) * scale, panels[6], 0.0f, UI::themeColors["primary"]);
	Sprite::DrawSprite(panels, glm::vec2(col, startY - 8) * scale, glm::vec2(width - col - col, endY - startY - 8) * scale, panels[8], 0.0f, UI::themeColors["primary"]);
	Sprite::DrawSprite(panels, glm::vec2(width - col, startY - 8) * scale, glm::vec2(col, endY - startY - 8) * scale, panels[7], 0.0f, UI::themeColors["primary"]);
	*/

	pos.y -= 8 * scale;
	for (int i = 0; i < shown; i++)
	{
		auto item = items->items[i + scroll];
		auto size = itemSize * scale;
		pos.y += (itemSpace * scale) + size - (itemSize * scale);
		if (i + scroll == highlight)
		{
			//auto offset = glm::vec2(item->type == DoomMenuItem::Type::Checkbox ? (checkboxOffset * scale) : 0, 0);
			auto highlightSize = Sprite::MeasureText(1, item->caption, itemSize * scale);
			if (item->type == DoomMenuItem::Type::Checkbox)
				highlightSize.x += checkboxOffset;
			highlightSize.x += hiliteBarOffset * scale;
			highlightSize.y *= partScale * 0.5f;
			Sprite::DrawSprite(controls, pos + glm::vec2(-(highlightSize.y) * scale, 0), glm::vec2(highlightSize.y), controls[7], 0, UI::themeColors["secondary"]);
			Sprite::DrawSprite(controls, pos + glm::vec2(highlightSize.x, 0), glm::vec2(highlightSize.y), controls[8], 0, UI::themeColors["secondary"]);
			Sprite::DrawSprite(controls, pos, highlightSize, controls[9], 0, UI::themeColors["secondary"]);
			break;
		}
	}
	pos.y = startY - (16 * scale);

	for (int i = 0; i < shown; i++)
	{
		auto item = items->items[i + scroll];
		auto color = UI::themeColors["white"];
		auto offset = glm::vec2(item->type == DoomMenuItem::Type::Checkbox ? (checkboxOffset * scale) : 0, 0);
		auto font = 1;
		auto size = itemSize * scale;

		if (item->type == DoomMenuItem::Type::Text)
		{
			font = item->selection;
			size = item->maxVal * scale;
		}
		else if (item->type == DoomMenuItem::Type::KeyBind && i + scroll == remapping)
		{
			color = UI::themeColors["yellow"];
		}

		Sprite::DrawText(font, item->caption, pos + offset, color, size);

		if (item->type == DoomMenuItem::Type::Options)
		{
			Sprite::DrawText(1, item->options[item->selection], pos + glm::vec2(col, 0), color, size);
		}
		else if (item->type == DoomMenuItem::Type::Slider)
		{
			if (item->format != nullptr)
			{
				auto fmt = item->format(item);
				Sprite::DrawText(1, fmt, pos + glm::vec2(col + col + (sliderValueOffsetX * scale), sliderValueOffsetY), color, size * partScale);
			}
		}
		else if (item->type == DoomMenuItem::Type::KeyBind)
		{
			auto key = Inputs.Keys[item->selection];
			Sprite::DrawText(1, key.Name, pos + glm::vec2(col, 0), color, size);
			
			Sprite::DrawText(1, key.GamepadButton == -1 ? "[none]" : GamepadPUAMap[key.GamepadButton], pos + glm::vec2(col * 2, 0), color, size);
		}

		itemY.push_back(pos.y);
		pos.y += (itemSpace * scale) + size - (itemSize * scale);
	}

	//terminator
	itemY.push_back(pos.y);

	if (items->DrawSpecial)
		items->DrawSpecial(items, items->items[highlight]);

	for (int i = 0; i < shown; i++)
	{
		auto item = i == 0 ? items->items[0] : items->items[i + scroll];
		const auto offset = glm::vec2(item->type == DoomMenuItem::Type::Checkbox ? (checkboxOffset * scale) : 0, 0);

		pos.y = itemY[i];

		if (item->type == DoomMenuItem::Type::Checkbox)
		{
			Sprite::DrawSprite(controls, pos + glm::vec2(0, 4 * scale), glm::vec2(partSize), controls[4], 0, UI::themeColors["secondary"]);
			if (item->selection)
				Sprite::DrawSprite(controls, pos + glm::vec2(0, 4 * scale), glm::vec2(partSize), controls[5], 0, UI::themeColors["white"]);
		}
		else if (item->type == DoomMenuItem::Type::Slider)
		{
			const auto color = UI::themeColors["white"];
			auto barLength = col;
			Sprite::DrawSprite(controls, pos + glm::vec2(col, 10 * scale), glm::vec2(partSize), controls[0], 0, color);
			Sprite::DrawSprite(controls, pos + glm::vec2(col + barLength + (partSize * 1), 10 * scale), glm::vec2(partSize), controls[1], 0, color);
			Sprite::DrawSprite(controls, pos + glm::vec2(col + partSize, 10 * scale), glm::vec2(barLength, partSize), controls[2], 0, color);

			sliderStart = pos.x + col + partSize;
			sliderEnd = sliderStart + barLength;

			//thanks GZDoom
			auto range = item->maxVal - item->minVal;
			auto ccur = glm::clamp(item->selection, item->minVal, item->maxVal) - item->minVal;
			auto thumbPos = partSize + ((ccur * (barLength - (partSize * 2))) / range);

			auto thumb = glm::vec2(col + (int)thumbPos, 10 * scale);
			Sprite::DrawSprite(controls, pos + thumb, thumbSize, controls[3], 0, color);
		}
	}

	buttonGuide.Draw();
}

//TODO: add DialogueChoiceMenu.
