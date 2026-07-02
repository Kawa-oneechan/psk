#include "NineSlicer.h"
#include "SpriteRenderer.h"
#include "Texture.h"

extern float scale;

NineSlicer::NineSlicer(const std::string& texture, int left, int top, int width, int height) : texture(VFS::GetTexture(texture)), Size(glm::vec2(width, height))
{
	parent = nullptr;
	Position = glm::vec2(left, top);
}

void NineSlicer::Draw(Texture& tex, const glm::vec2& position, glm::vec2& size, float scale, const glm::vec4& color)
{
	auto minWidth = (tex[0].z + tex[2].z) * scale;
	auto minHeight = (tex[0].w + tex[6].w) * scale;
	if (size.x < minWidth)
		size.x = minWidth;
	if (size.y < minHeight)
		size.y = minHeight;

	auto _size = glm::vec2(tex[0].z, tex[0].w) * scale;
	auto left = position.x;
	auto top = position.y;
	auto width = size.x;
	auto height = size.y;
	auto right = left + width - _size.x;
	auto bottom = top + height - _size.y;
	auto edgeWidth = (width - _size.x - _size.x);
	auto edgeHeight = (height - _size.y - _size.y);

	//Top left
	Sprite::DrawSprite(tex, glm::vec2(left, top), _size, tex[0], 0.0f, color);

	//Top
	if (width > minWidth)
		Sprite::DrawSprite(tex, glm::vec2(left + _size.x, top), glm::vec2(edgeWidth, _size.y), tex[1], 0.0f, color);

	//Top right
	Sprite::DrawSprite(tex, glm::vec2(right, top), _size, tex[2], 0.0f, color);

	//Left
	if (height > minHeight)
		Sprite::DrawSprite(tex, glm::vec2(left, top + _size.y), glm::vec2(_size.x, edgeHeight), tex[3], 0.0f, color);

	//Fill
	if (width > minWidth && height > minHeight)
		Sprite::DrawSprite(tex, glm::vec2(left + _size.x, top + _size.y), glm::vec2(edgeWidth, edgeHeight), tex[4], 0.0f, color);

	//Right
	if (height > minHeight)
		Sprite::DrawSprite(tex, glm::vec2(right, top + _size.y), glm::vec2(_size.x, edgeHeight), tex[5], 0.0f, color);

	//Bottom left
	Sprite::DrawSprite(tex, glm::vec2(left, bottom), _size, tex[6], 0.0f, color);

	//Bottom
	if (width > minWidth)
		Sprite::DrawSprite(tex, glm::vec2(left + _size.x, bottom), glm::vec2(edgeWidth, _size.y), tex[7], 0.0f, color);

	//Bottom right
	Sprite::DrawSprite(tex, glm::vec2(right, bottom), _size, tex[8], 0.0f, color);
}

void NineSlicer::Draw(float dt)
{
	(void)(dt);
	auto& tex = *texture;

	Draw(tex, AbsolutePosition, Size, Scale, Color);

	Tickable2D::Draw(dt);
}
