#include "NineSlicer.h"
#include "SpriteRenderer.h"

NineSlicer::NineSlicer(const std::string& texPath, int left, int top, int width, int height) : Position(left, top), Size(width, height)
{
	texture = std::make_shared<Texture>(texPath);
}

void NineSlicer::Draw(float dt)
{
	dt;
	auto& tex = *texture;

	auto minWidth = (tex[0].z + tex[2].z) * Scale;
	auto minHeight = (tex[0].w + tex[6].w) * Scale;
	if (Size.x < minWidth)
		Size.x = minWidth;
	if (Size.y < minHeight)
		Size.y = minHeight;

	auto size = glm::vec2(tex[0].z, tex[0].w) * Scale;
	auto left = Position.x;
	auto top = Position.y;
	auto width = Size.x;
	auto height = Size.y;
	auto right = left + width - size.x;
	auto bottom = top + height - size.y;
	auto edgeWidth = (width - size.x - size.x);
	auto edgeHeight = (height - size.y - size.y);

	//Top left
	Sprite::DrawSprite(tex, glm::vec2(left, top), size, tex[0], 0.0f, Color);

	//Top
	if (width > minWidth)
		Sprite::DrawSprite(tex, glm::vec2(left + size.x, top), glm::vec2(edgeWidth, size.y), tex[1], 0.0f, Color);

	//Top right
	Sprite::DrawSprite(tex, glm::vec2(right, top), size, tex[2], 0.0f, Color);

	//Left
	if (height > minHeight)
		Sprite::DrawSprite(tex, glm::vec2(left, top + size.y), glm::vec2(size.x, edgeHeight), tex[3], 0.0f, Color);

	//Fill
	if (width > minWidth && height > minHeight)
		Sprite::DrawSprite(tex, glm::vec2(left + size.x, top + size.y), glm::vec2(edgeWidth, edgeHeight), tex[4], 0.0f, Color);

	//Right
	if (height > minHeight)
		Sprite::DrawSprite(tex, glm::vec2(right, top + size.y), glm::vec2(size.x, edgeHeight), tex[5], 0.0f, Color);

	//Bottom left
	Sprite::DrawSprite(tex, glm::vec2(left, bottom), size, tex[6], 0.0f, Color);

	//Bottom
	if (width > minWidth)
		Sprite::DrawSprite(tex, glm::vec2(left + size.x, bottom), glm::vec2(edgeWidth, size.y), tex[7], 0.0f, Color);

	//Bottom right
	Sprite::DrawSprite(tex, glm::vec2(right, bottom), size, tex[8], 0.0f, Color);
}
