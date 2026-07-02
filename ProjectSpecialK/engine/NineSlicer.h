#pragma once
#include "Tickable.h"

class NineSlicer : public Tickable2D
{
private:
	TextureP texture;

public:
	glm::vec2 Size;
	glm::vec4 Color{ 1, 1, 1, 1 };
	float Scale{ 1 };

	NineSlicer(const std::string& texture, int left, int top, int width, int height);
	void Draw(float dt) override;

	static void Draw(Texture& tex, const glm::vec2& position, glm::vec2& size, float scale, const glm::vec4& color);
};
