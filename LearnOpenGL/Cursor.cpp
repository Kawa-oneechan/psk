#include "Cursor.h"
#include "InputsMap.h"

Cursor::Cursor()
{
	hand = new Texture("ui/cursors.png");
	GetAtlas(atlas, "ui/cursors.json");
	//atlas = new _TextureAtlas("ui/cursors.json");

	auto hsj = ReadJSON("ui/cursors.json")->AsObject();
	for (auto& hs : hsj["hotspots"]->AsArray())
		hotspots.push_back(GetJSONVec2(hs));

	SetScale(100);
	Select(0);
	size = glm::vec2(frame.w);
}

void Cursor::Select(int style)
{
	frame = atlas[style];
	hotspot = hotspots[style];
}

void Cursor::SetScale(int newScale)
{
	scale = newScale / 100.0f;
	size = glm::vec2(frame.w * scale);
}

void Cursor::Draw()
{
	sprender->DrawSprite(hand, Inputs.MousePosition - (hotspot * scale), size, frame);
}

