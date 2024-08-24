#include "ButtonGuide.h"

static constexpr auto padding = 32.0f;
static constexpr auto margin = 32.0f;

void ButtonGuide::SetButtons(std::initializer_list<std::string> labels)
{
	texts.clear();
	highlight = -1;
	left = width;
	for (auto l : labels)
	{
		if (l[0] == '!') //important
		{
			highlight = (int)texts.size();
			l = l.substr(1);
		}
		texts.push_back(l);
		auto width = sprender->MeasureText(1, l, 100.0f).x + padding;
		widths.push_back(width);
		left -= width + (margin / 2);
	}
}

void ButtonGuide::Draw()
{
	auto pillHeight = 24; //TODO
	auto pos = glm::vec2(left, height - margin - pillHeight);
	for (int i = 0; i < texts.size(); i++)
	{
		//TODO: draw pill
		sprender->DrawText(1, texts[i], pos, i == highlight ? UI::themeColors["keybarhf"] : UI::themeColors["keybarf"]);
		pos.x += widths[i];
	}
}
