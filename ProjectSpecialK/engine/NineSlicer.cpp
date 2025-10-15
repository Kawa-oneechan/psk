#include "NineSlicer.h"
#include "SpriteRenderer.h"
#include "Texture.h"

extern float scale;

NineSlicer::NineSlicer(const std::string& texPath, int left, int top, int width, int height) : texture(std::make_shared<Texture>(texPath)), Size(glm::vec2(width, height))
{
	parent = nullptr;
	Position = glm::vec2(left, top);
}

void NineSlicer::Draw(float dt)
{
	dt;
	auto& tex = *texture;

	auto sc = (Scale > 0) ? Scale : scale;

	auto minWidth = (tex[0].z + tex[2].z) * sc;
	auto minHeight = (tex[0].w + tex[6].w) * sc;
	if (Size.x < minWidth)
		Size.x = minWidth;
	if (Size.y < minHeight)
		Size.y = minHeight;
	auto sSize = Size * sc;

	auto size = glm::vec2(tex[0].z, tex[0].w) * sc;
	auto left = AbsolutePosition.x;
	auto top = AbsolutePosition.y;
	auto width = sSize.x;
	auto height = sSize.y;
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

	Tickable2D::Draw(dt);
}
