#pragma once
#include <memory>
#include "Texture.h"

class Cursor
{
private:
	Texture hand{ Texture("ui/cursors.png") };
	std::vector<glm::vec2> hotspots;
	glm::vec2 hotspot;
	glm::vec4 frame;
	glm::vec2 size;
	glm::vec4 penFrame;
	float scale;
	bool rotate{ false };

public:
	glm::vec4 Pen{ 1.0, 0.0, 0.0, 1.0 };

	Cursor();
	void Select(int style);
	void SetScale(int newScale);
	void Draw();
};

using CursorP = std::shared_ptr<Cursor>;

extern CursorP cursor;
