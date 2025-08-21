#include "NineSlicer.h"
#include "SpriteRenderer.h"

NineSlicer::NineSlicer(const std::string& texture, int left, int top, int width, int height) : left(left), top(top), width(width), height(height)
{
	this->texture = Texture(texture);
}

void NineSlicer::Draw(float dt)
{
	dt;

	auto size = glm::vec2(texture[0].z, texture[0].w);
	auto right = left + width - size.x;
	auto bottom = top + height - size.y;
	auto edgeWidth = width - size.x - size.x;
	auto edgeHeight = height - size.y - size.y;

	//Top left
	Sprite::DrawSprite(texture, glm::vec2(left, top), size, texture[0]);

	//Top
	Sprite::DrawSprite(texture, glm::vec2(left + size.x, top), glm::vec2(edgeWidth, size.y), texture[1]);

	//Top right
	Sprite::DrawSprite(texture, glm::vec2(right, top), size, texture[2]);

	//Left
	Sprite::DrawSprite(texture, glm::vec2(left, top + size.y), glm::vec2(size.x, edgeHeight), texture[3]);

	//Fill
	Sprite::DrawSprite(texture, glm::vec2(left + size.x, top + size.y), glm::vec2(edgeWidth, edgeHeight), texture[4]);

	//Right
	Sprite::DrawSprite(texture, glm::vec2(right, top + size.y), glm::vec2(size.x, edgeHeight), texture[5]);

	//Bottom left
	Sprite::DrawSprite(texture, glm::vec2(left, bottom), size, texture[6]);

	//Bottom
	Sprite::DrawSprite(texture,glm::vec2(left + size.x, bottom), glm::vec2(edgeWidth, size.y), texture[7]);

	//Bottom right
	Sprite::DrawSprite(texture, glm::vec2(right, bottom), size, texture[8]);
}
