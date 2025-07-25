#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Texture.h"
#include "Shader.h"

namespace Sprite
{
	constexpr int BatchSize = 200;

	enum SpriteFlags
	{
		NoFlags = 0,
		FlipX = 1,
		FlipY = 2,
		FlipXY = 3,
		TopLeft = 4,
	};

	void DrawSprite(Shader* shader, Texture& texture, glm::vec2 position, glm::vec2 size, glm::vec4 srcRect = glm::vec4(0.0f), float rotate = 0.0f, const glm::vec4& color = glm::vec4(1), SpriteFlags flags = SpriteFlags::NoFlags);
	void DrawSprite(Texture& texture, const glm::vec2& position, const glm::vec2& size, const glm::vec4& srcRect = glm::vec4(0.0f), float rotate = 0.0f, const glm::vec4& color = glm::vec4(1), SpriteFlags flags = SpriteFlags::NoFlags);
	void DrawSprite(Shader* shader, Texture& texture, glm::vec2 position);
	void DrawSprite(Texture& texture, glm::vec2 position);
	void DrawLine(const glm::vec2& from, const glm::vec2& to, const glm::vec4& color);

	void DrawText(int font, const std::string& text, glm::vec2 position, const glm::vec4& color = glm::vec4(1), float size = 100, float angle = 0.0f, bool raw = false);
	void DrawText(const std::string& text, const glm::vec2& position, const glm::vec4& color = glm::vec4(1), float size = 100, float angle = 0.0f, bool raw = false);
	glm::vec2 MeasureText(int font, const std::string& text, float size, bool raw = false);

	void FlushBatch();
}
