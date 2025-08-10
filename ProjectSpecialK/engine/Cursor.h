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
	//Index of the cursor style that depicts an hourglass. This cursor
	//will be animated to slowly spin.
	int WaitIndex{ 1 };
	//Lowest index, inclusive, of the cursor styles that have colored parts.
	int PenMinIndex{ 11 };
	//Highest index, inclusive, of the cursor styles that have colored parts.
	int PenMaxIndex{ 15 };
	//How many frames to add to get to the colored parts for colored cursors.
	int	PenOffset{ 8 };
	//The color to use for colored cursors.
	glm::vec4 Pen{ 1.0, 0.0, 0.0, 1.0 };

	Cursor();
	//Sets up the cursor to a specific style.
	void Select(int style);
	//Sets the scale of the cursor as a percentage float from 0.2 to 10.0.
	void SetScale(float newScale);
	//Draws the cursor to the screen.
	void Draw();
};

using CursorP = std::shared_ptr<Cursor>;

extern CursorP cursor;
