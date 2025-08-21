#pragma once
#include <glm/glm.hpp>
#include "Texture.h"

class DropLabel
{
public:
	enum class Style
	{
		Blur, Drop
	};

private:
	std::string text;
	glm::vec2 size;
	Texture canvas;
	float textSize;
	int font;
	Style style;

	void update();

public:

	DropLabel(const std::string& text, int font = 1, float size = 100.0f, Style style = Style::Blur);
	void SetText(const std::string& text);
	const glm::vec2 Size() const { return size; }
	Texture& Texture() { return canvas; }
};
