#pragma once
#include <string>
#include <glm/glm.hpp>

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
	glm::vec4 color;
	 Texture canvas;
	float textSize;
	int font;
	Style style;

	void update();

public:
	explicit DropLabel(const std::string& text, int font = 1, float size = 100.0f, glm::vec4 color = glm::vec4(-1), Style style = Style::Blur);
	//Changes the text of the DropLabel, causing a refresh of the underlying texture.
	void SetText(const std::string& text);
	const glm::vec2 Size() const { return size; }
	//Returns a reference to the underlying texture.
	class Texture& Texture() { return canvas; }
};
