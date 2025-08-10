#include "Cursor.h"
#include "InputsMap.h"
#include "VFS.h"
#include "JsonUtils.h"
#include "SpriteRenderer.h"

extern "C" { double glfwGetTime(void); }

Cursor::Cursor()
{
	auto doc = VFS::ReadJSON("ui/cursors.json");
	auto hsj = doc.as_object();
	for (auto& hs : hsj["hotspots"].as_array())
		hotspots.push_back(GetJSONVec2(hs));

	SetScale(100);
	Select(0);
	size = glm::vec2(frame.w);
}

void Cursor::Select(int style)
{
	frame = hand[style];
	hotspot = hotspots[style];
	rotate = (style == 1);
	penFrame = glm::vec4(-1);
	if (style >= 11 && style <= 15)
		penFrame = hand[style + 8];
}

void Cursor::SetScale(int newScale)
{
	scale = newScale / 100.0f;
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
