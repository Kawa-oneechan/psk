#pragma once
#include <string>
#include <glm/glm.hpp>
#include "Types.h"

class Texture;
class Shader;

namespace Sprite
{
	//Number of sprites to queue up at once.
	constexpr int BatchSize = 200;

	enum SpriteFlags
	{
		NoFlags = 0, //Rotate along the center, position is the top left.
		FlipX = 1, //Draw the sprite horizontally flipped.
		FlipY = 2, //Draw the sprite vertically flipped.
		FlipXY = 3, //Draw the sprite flipped bothways.
		RotateCenter = 0, //Rotate the sprite along the center.
		RotateTopLeft = 4, //Rorate the sprite along the top left corner.
		TopLeftOrigin = 0, //Consider the position to be the top left of the sprite.
		CenterOrigin = 8, //Consider the position to be the center of the sprite.
		MidBotOrigin = 16, //Consider the position to be the bottom middle of the sprite.
		NoClip = 32, //Don't try to clip the sprite. Good if it's rotated.
	};

	void DrawSprite(Shader* shader, Texture& texture, glm::vec2 position, glm::vec2 size, glm::vec4 srcRect = glm::vec4(0.0f), float rotate = 0.0f, const glm::vec4& color = glm::vec4(1), SpriteFlags flags = SpriteFlags::NoFlags);
	void DrawSprite(Texture& texture, const glm::vec2& position, const glm::vec2& size, const glm::vec4& srcRect = glm::vec4(0.0f), float rotate = 0.0f, const glm::vec4& color = glm::vec4(1), SpriteFlags flags = SpriteFlags::NoFlags);
	void DrawSprite(Shader* shader, Texture& texture, const glm::vec2& position, const glm::vec4& srcRect, float rotate = 0.0f, const glm::vec4& color = glm::vec4(1), SpriteFlags flags = SpriteFlags::NoFlags);
	void DrawSprite(Texture& texture, const glm::vec2& position, const glm::vec4& srcRect, float rotate = 0.0f, const glm::vec4& color = glm::vec4(1), SpriteFlags flags = SpriteFlags::NoFlags);
	void DrawSprite(Shader* shader, Texture& texture, glm::vec2 position);
	void DrawSprite(Texture& texture, glm::vec2 position);
	void DrawLine(const glm::vec2& from, const glm::vec2& to, const glm::vec4& color);
	void DrawRect(const glm::vec4& bounds, const glm::vec4& color);
	void DrawCircle(const glm::vec2& center, float radius, int segments, const glm::vec4& color);
	void DrawPoly(const polygon& poly, const glm::vec2& origin, float scale, const glm::vec4& color);

	void DrawText(int font, const std::string& text, glm::vec2 position, const glm::vec4& color = glm::vec4(1), float size = 100, float angle = 0.0f, bool raw = false);
	void DrawText(const std::string& text, const glm::vec2& position, const glm::vec4& color = glm::vec4(1), float size = 100, float angle = 0.0f, bool raw = false);
	glm::vec2 MeasureText(int font, const std::string& text, float size, bool raw = false);

	void FlushBatch();
}
