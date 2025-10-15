#pragma once
#include "Tickable2D.h"

class Texture;

class NineSlicer : public Tickable2D
{
private:
	std::shared_ptr<Texture> texture;

public:
	glm::vec2 Size;
	glm::vec4 Color{ 1, 1, 1, 1 };

	NineSlicer(const std::string& texture, int left, int top, int width, int height);
	void Draw(float dt) override;
};
