#pragma once

#include "support/glad/glad.h"
#include "support/glm/glm.hpp"
#include "support/glm/gtc/matrix_transform.hpp"

#include "Texture.h"
#include "Shader.h"

constexpr int FontAtlasExtent = 512;
constexpr int MaxFonts = 4;

//Surely there's a better way to define these functions without having to type this whole damn thing?
#define MSBTParams const std::vector<std::string>& tags, int start, int len

enum SpriteFlags
{
	NoFlags = 0,
	FlipX = 1,
	FlipY = 2,
	FlipXY = 3,
	TopLeft = 4,
};

class SpriteRenderer
{
public:
	SpriteRenderer();
	~SpriteRenderer();
	void DrawSprite(Shader* shader, Texture& texture, glm::vec2 position, glm::vec2 size, glm::vec4 srcRect = glm::vec4(0.0f), float rotate = 0.0f, const glm::vec4& color = glm::vec4(1), SpriteFlags flags = SpriteFlags::NoFlags);
	void DrawSprite(Texture& texture, const glm::vec2& position, const glm::vec2& size, const glm::vec4& srcRect = glm::vec4(0.0f), float rotate = 0.0f, const glm::vec4& color = glm::vec4(1), SpriteFlags flags = SpriteFlags::NoFlags);
	void DrawSprite(Shader* shader, Texture& texture, glm::vec2 position);
	void DrawSprite(Texture& texture, glm::vec2 position);
	void SpriteRenderer::DrawLine(const glm::vec2& from, const glm::vec2& to, const glm::vec4& color);

	//In TextRenderer.cpp:
	void DrawText(int font, const std::string& text, glm::vec2 position, const glm::vec4& color = glm::vec4(1), float size = 100, float angle = 0.0f, bool raw = false);
	void DrawText(const std::string& text, const glm::vec2& position, const glm::vec4& color = glm::vec4(1), float size = 100, float angle = 0.0f, bool raw = false);
	glm::vec2 SpriteRenderer::MeasureText(int font, const std::string& text, float size, bool raw = false);

	void Flush();

private:
	unsigned int quadVAO;
	glm::vec4 textRenderColor, originalTextRenderColor;
	float textRenderSize, originalTextRenderSize;
	int textRenderFont, originalTextRenderFont;

	Shader* fontShader;

	void msbtColor(MSBTParams);
	void msbtSize(MSBTParams);
	void msbtFont(MSBTParams);

	typedef void(SpriteRenderer::*MSBTFunc)(MSBTParams);
	const std::map<std::string, MSBTFunc> msbtPhase3
	{
		{ "color", &SpriteRenderer::msbtColor },
		{ "/color", &SpriteRenderer::msbtColor },
		{ "size", &SpriteRenderer::msbtSize },
		{ "/size", &SpriteRenderer::msbtSize },
		{ "font", &SpriteRenderer::msbtFont },
		{ "/font", &SpriteRenderer::msbtFont },
	};

	void LoadFontBank(int font, int bank);
};
