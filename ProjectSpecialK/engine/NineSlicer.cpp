#include "NineSlicer.h"
#include "SpriteRenderer.h"

NineSlicer::NineSlicer(const std::string& texPath, int left, int top, int width, int height) : left(left), top(top), width(width), height(height)
{
	texture = std::make_shared<Texture>(texPath);
}

void NineSlicer::Draw(float dt)
{
	dt;

	auto& tex = *texture;

	auto size = glm::vec2(tex[0].z, tex[0].w);
	auto right = left + width - size.x;
	auto bottom = top + height - size.y;
	auto edgeWidth = width - size.x - size.x;
	auto edgeHeight = height - size.y - size.y;

	//Top left
	Sprite::DrawSprite(tex, glm::vec2(left, top), size, tex[0]);

	//Top
	Sprite::DrawSprite(tex, glm::vec2(left + size.x, top), glm::vec2(edgeWidth, size.y), tex[1]);

	//Top right
	Sprite::DrawSprite(tex, glm::vec2(right, top), size, tex[2]);

	//Left
	Sprite::DrawSprite(tex, glm::vec2(left, top + size.y), glm::vec2(size.x, edgeHeight), tex[3]);

	//Fill
	Sprite::DrawSprite(tex, glm::vec2(left + size.x, top + size.y), glm::vec2(edgeWidth, edgeHeight), tex[4]);

	//Right
	Sprite::DrawSprite(tex, glm::vec2(right, top + size.y), glm::vec2(size.x, edgeHeight), tex[5]);

	//Bottom left
	Sprite::DrawSprite(tex, glm::vec2(left, bottom), size, tex[6]);

	//Bottom
	Sprite::DrawSprite(tex, glm::vec2(left + size.x, bottom), glm::vec2(edgeWidth, size.y), tex[7]);

	//Bottom right
	Sprite::DrawSprite(tex, glm::vec2(right, bottom), size, tex[8]);
}
