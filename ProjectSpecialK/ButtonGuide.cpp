#include "ButtonGuide.h"

static constexpr auto padding = 32.0f;
static constexpr auto margin = 32.0f;

struct button
{
	std::string text;
	float width;
	bool important;
};

std::vector<button> buttons;

void ButtonGuide::SetButtons(std::initializer_list<std::string> labels)
{
	buttons.clear();

	for (auto l : labels)
	{
		bool important = false;
		if (l[0] == '!')
		{
			l = l.substr(1);
			important = true;
		}
		auto width = Sprite::MeasureText(1, l, 90.0f).x + padding;
		buttons.push_back({ l, width, important });
	}
}

void ButtonGuide::Draw()
{
	auto s = scale * 1.40f;
	auto& controls = *UI::controls;
	auto pillWidth = controls[7].z * s;
	auto pillHeight = controls[7].w * s;
	auto pillSize = glm::vec2(pillWidth, pillHeight);

	auto pos = glm::vec2(width - (margin * 1.0f), height - margin - pillHeight);

	for (int i = (int)buttons.size() - 1; i > -1; i--)
	{
		pos.x -= buttons[i].width + (margin  * 0.5f);
		auto f = pos;

		const auto color = buttons[i].important ? UI::themeColors["keybarh"] : UI::themeColors["keybar"];
		Sprite::DrawSprite(controls, f, pillSize, controls[7], 0, color);
		f.x += pillWidth;
		Sprite::DrawSprite(controls, f, glm::vec2(buttons[i].width - (pillWidth * 1.5f), pillHeight), controls[9], 0, color);
		f.x += buttons[i].width - (pillWidth * 1.5f);
		Sprite::DrawSprite(controls, f, pillSize, controls[8], 0, color);
		
		f = pos + glm::vec2(18 * s, 9 * s);
		Sprite::DrawText(1, buttons[i].text, f, buttons[i].important ? UI::themeColors["keybarhf"] : UI::themeColors["keybarf"], 90.0f * scale);
	}
}
