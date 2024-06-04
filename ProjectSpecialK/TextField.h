#pragma once

#include "SpecialK.h"

class TextField : public Tickable
{
	//TODO: since we have a scissor test, why not have scrolling?
	//As in, [lo, world!_ ] with the "hel" off-screen, via an offset?

public:
	glm::vec4 rect;
	glm::vec4 color;
	int font;
	float size;
	std::string value;
	size_t caret;

	TextField();
	void Draw(double dt);
	bool Character(unsigned int codepoint);
	void Clear();
	void Set(const std::string& to);
};
