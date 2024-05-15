#pragma once

#include "support/glad/glad.h"
#include "support/glm/glm.hpp"
#include "support/glm/gtc/matrix_transform.hpp"

#include "Texture.h"
#include "Shader.h"

#define FONTATLAS_SIZE 512
#define MAXFONTS 4


#define MSBTParams const std::vector<std::string>& tags, int start, int len

class SpriteRenderer
{
public:
	SpriteRenderer();
	~SpriteRenderer();
	void DrawSprite(Shader& shader, Texture& texture, glm::vec2 position, glm::vec2 size, glm::vec4 srcRect = glm::vec4(0.0f), float rotate = 0.0f, const glm::vec4& color = glm::vec4(1), int flip = 0);
	void DrawSprite(Texture& texture, const glm::vec2& position, const glm::vec2& size, const glm::vec4& srcRect = glm::vec4(0.0f), float rotate = 0.0f, const glm::vec4& color = glm::vec4(1), int flip = 0);
	void DrawSprite(Shader& shader, Texture& texture, glm::vec2 position);
	void DrawSprite(Texture& texture, glm::vec2 position);

	void DrawText(int font, const std::string& text, glm::vec2 position, const glm::vec4& color = glm::vec4(1), float size = 100, float angle = 0.0f);
	void DrawText(const std::string& text, const glm::vec2& position, const glm::vec4& color = glm::vec4(1), float size = 100, float angle = 0.0f);

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
