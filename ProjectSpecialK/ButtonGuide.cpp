#include "ButtonGuide.h"

static constexpr auto padding = 32.0f;
static constexpr auto margin = 32.0f;

void ButtonGuide::SetButtons(std::initializer_list<std::string> labels)
{
	texts.clear();
	widths.clear();
	highlight = -1;
	left = (float)width;
	for (auto l : labels)
	{
		if (l[0] == '!') //important
		{
			highlight = (int)texts.size();
			l = l.substr(1);
		}
		texts.push_back(l);
		auto width = (Sprite::MeasureText(1, l, 100.0f).x + padding);
		widths.push_back(width);
		left -= width + (margin / 2);
	}
}

void ButtonGuide::Draw()
{
	auto s = scale * 1.40f;
	auto& controls = *UI::controls;
	auto pillWidth = controls[7].z * s;
	auto pillHeight = controls[7].w * s;
	auto pillSize = glm::vec2(pillWidth, pillHeight);
	std::vector<glm::vec2> lefts;

	auto pos = glm::vec2(left, height - margin - pillHeight);

	for (int i = 0; i < texts.size(); i++)
	{
		lefts.push_back(pos);
		const auto color = i == highlight ? UI::themeColors["keybarh"] : UI::themeColors["keybar"];
		Sprite::DrawSprite(controls, pos, pillSize, controls[7], 0, color);
		pos.x += pillWidth;
		Sprite::DrawSprite(controls, pos, glm::vec2(widths[i] - (pillWidth * 1.5f), pillHeight), controls[9], 0, color);
		pos.x += widths[i] - (pillWidth * 1.5f);
		Sprite::DrawSprite(controls, pos, pillSize, controls[8], 0, color);
		pos.x += margin / 1.5f;
	}

	for (int i = 0; i < texts.size(); i++)
	{
		pos = lefts[i] + glm::vec2(18 * s, 9 * s);
		Sprite::DrawText(1, texts[i], pos, i == highlight ? UI::themeColors["keybarhf"] : UI::themeColors["keybarf"], 90.0f * scale);
	}
}
