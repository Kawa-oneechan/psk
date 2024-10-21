#include "Cursor.h"
#include "InputsMap.h"

Cursor::Cursor()
{
	auto doc = VFS::ReadJSON("ui/cursors.json");
	auto hsj = doc->AsObject();
	for (auto& hs : hsj["hotspots"]->AsArray())
		hotspots.push_back(GetJSONVec2(hs));
	delete doc;

	SetScale(100);
	Select(0);
	size = glm::vec2(frame.w);
}

void Cursor::Select(int style)
{
	frame = hand[style];
	hotspot = hotspots[style];
	rotate = (style == 1);
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
}
