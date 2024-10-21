#pragma once

#include "SpecialK.h"

class Cursor
{
private:
	Texture hand{ Texture("ui/cursors.png") };
	std::vector<glm::vec2> hotspots;
	glm::vec2 hotspot;
	glm::vec4 frame;
	glm::vec2 size;
	float scale;
	bool rotate{ false };

public:
	Cursor();
	void Select(int style);
	void SetScale(int newScale);
	void Draw();
};

using CursorP = std::shared_ptr<Cursor>;

extern CursorP cursor;
