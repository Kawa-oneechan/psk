#include <algorithm>
#include "Cursor.h"
#include "InputsMap.h"
#include "VFS.h"
#include "JsonUtils.h"
#include "SpriteRenderer.h"

extern "C" { double glfwGetTime(void); }

Cursor::Cursor()
{
	auto doc = VFS::ReadJSON("ui/cursors.json");
	auto spots = doc.as_object()["hotspots"].as_array();
	std::for_each(spots.cbegin(), spots.cend(), [&](auto h)
	{
		hotspots.push_back(GetJSONVec2(h));
	});

	SetScale(1.0);
	Select(0);
	size = glm::vec2(frame.w);
}

void Cursor::Select(int style)
{
	style = glm::clamp(style, 0, (int)hotspots.size());
	frame = hand[style];
	hotspot = hotspots[style];
	rotate = (style == WaitIndex);
	penFrame = glm::vec4(-1);
	if (style >= PenMinIndex && style <= PenMaxIndex)
		penFrame = hand[style + PenOffset];
}

void Cursor::Select(const std::string& style)
{
	auto& atlas = hand.Atlas();
	auto it = atlas.names.find(style);
	if (it == atlas.names.cend())
		Select(0);
	else
		Select(it->second);
}

void Cursor::SetScale(float newScale)
{
	scale = glm::clamp(newScale, 0.2f, 10.f);
	size = glm::vec2(frame.w * scale);
}

void Cursor::Draw()
{
	float rotation = 0;
	if (rotate)
	{
		auto time = (float)glfwGetTime();
		rotation = time * glm::radians(3000.0f);
	}
	Sprite::DrawSprite(hand, Inputs.MousePosition - (hotspot * scale), size, frame, rotation);
	if (penFrame.x != -1)
		Sprite::DrawSprite(hand, Inputs.MousePosition - (hotspot * scale), size, penFrame, rotation, Pen);
}
