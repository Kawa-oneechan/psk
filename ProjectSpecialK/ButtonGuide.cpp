#include <vector>
#include "engine/SpriteRenderer.h"
#include "engine/Utilities.h"
#include "ButtonGuide.h"
#include "Types.h"
#include "Game.h"

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

	auto metrics = UI::json["metrics"].as_object();
	const float padding = metrics["buttonGuidePadding"].as_number();

	for (auto l : labels)
	{
		bool important = false;
		if (l[0] == '!')
		{
			l = l.substr(1);
			important = true;
		}
		auto thisWidth = Sprite::MeasureText(1, l, 90.0f).x + padding;
		buttons.push_back({ l, thisWidth, important });
	}
}

void ButtonGuide::Draw()
{
	auto s = scale * 1.40f;
	auto& controls = *UI::controls;
	auto pillWidth = controls[7].z * s;
	auto pillHeight = controls[7].w * s;
	auto pillSize = glm::vec2(pillWidth, pillHeight);

	auto metrics = UI::json["metrics"].as_object();
	const float margin = metrics["buttonGuideMargin"].as_number();
	const float edgeX = metrics["buttonGuideEdgeX"].as_number();
	const float edgeY = metrics["buttonGuideEdgeY"].as_number();

	auto pos = glm::vec2(width - edgeX, height - edgeY - pillHeight);

	for (int i = (int)buttons.size() - 1; i > -1; i--)
	{
		pos.x -= buttons[i].width + margin;
		auto f = pos;

		const auto color = buttons[i].important ? UI::themeColors["keybarh"] : UI::themeColors["keybar"];
		Sprite::DrawSprite(controls, f, pillSize, controls[10], 0, color);
		f.x += pillWidth;
		Sprite::DrawSprite(controls, f, glm::vec2(buttons[i].width - (pillWidth * 1.5f), pillHeight), controls[12], 0, color);
		f.x += buttons[i].width - (pillWidth * 1.5f);
		Sprite::DrawSprite(controls, f, pillSize, controls[11], 0, color);
		
		f = pos + glm::vec2(18 * s, 9 * s);
		Sprite::DrawText(1, buttons[i].text, f, buttons[i].important ? UI::themeColors["keybarhf"] : UI::themeColors["keybarf"], 90.0f * scale);
	}
}
