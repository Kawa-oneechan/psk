#pragma once
#include <glm/glm.hpp>
#include "Tickable.h"

class TextField : public Tickable
{
	//TODO: since we have a scissor test, why not have scrolling?
	//As in, [lo, world!_ ] with the "hel" off-screen, via an offset?

public:
	glm::vec4 rect{ glm::vec4(0,0,128,32) };
	glm::vec4 color{ glm::vec4(1) };
	int font{ 1 };
	float size{ 100.0 };
	std::string value;
	size_t caret{ 0 };
	float time{ 0 };

	void Draw(float dt) override;
	bool Character(unsigned int codepoint) override;
	bool Scancode(unsigned int scancode) override;
	void Clear();
	void Set(const std::string& to);
};
