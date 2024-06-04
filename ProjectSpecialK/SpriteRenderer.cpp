#include <string>
#include <sstream>

#include "SpriteRenderer.h"
#include "support/stb_truetype.h"
#include "support/stb_image_write.h"

extern Shader* spriteShader;
extern float width, height;
namespace UI
{
	extern glm::vec4 primaryColor;
	extern glm::vec4 secondaryColor;
	extern std::vector<glm::vec4> textColors;
	extern JSONObject& json;
};

static Texture* currentTexture = nullptr;
static Shader* currentShader = nullptr;

static glm::mat4 models[200];
static glm::vec4 sourceRects[200];
static glm::vec4 spriteColors[200];
static int spriteFlipX[200];
static int spriteFlipY[200];
static int instanceCursor = 0;

SpriteRenderer::SpriteRenderer()
{
	unsigned int VBO;
	float vertices[] = {
		//pos	tex
		0, 1,	0, -1,	//bottom left
		1, 0,	1,  0,	//top right
		0, 0,	0,  0,	//top left

		0, 1,	0, -1,	//bottom left
		1, 1,	1, -1,	//bottom right
		1, 0,	1,  0,	//top right
	};

	glGenVertexArrays(1, &this->quadVAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(this->quadVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	fontShader = new Shader("shaders/font.fs");

	originalTextRenderSize = textRenderSize = 100;
	originalTextRenderColor = textRenderColor = UI::textColors[0];
	originalTextRenderFont = textRenderFont = 0;
}

SpriteRenderer::~SpriteRenderer()
{
	glDeleteVertexArrays(1, &this->quadVAO);
}

void SpriteRenderer::Flush()
{
	if (instanceCursor == 0)
		return;

	currentShader->Use();
	currentTexture->Use(0);

	//shader.SetMat4("model", model);
	//shader.SetVec4("sourceRect", srcRect);
	//shader.SetVec4("spriteColor", color);
	//shader.SetBool("flipX", (flip & 1) == 1);
	//shader.SetBool("flipY", (flip & 2) == 2);
	glUniformMatrix4fv(glGetUniformLocation(currentShader->ID, "model"), instanceCursor, GL_FALSE, &models[0][0][0]);
	glUniform4fv(glGetUniformLocation(currentShader->ID, "sourceRect"), instanceCursor, &sourceRects[0][0]);
	glUniform4fv(glGetUniformLocation(currentShader->ID, "spriteColor"), instanceCursor, &spriteColors[0][0]);
	glUniform1iv(glGetUniformLocation(currentShader->ID, "flipX"), instanceCursor, &spriteFlipX[0]);
	glUniform1iv(glGetUniformLocation(currentShader->ID, "flipY"), instanceCursor, &spriteFlipY[0]);

	glBindVertexArray(this->quadVAO);
	//glDrawArrays(GL_TRIANGLES, 0, 6);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, instanceCursor);
	glBindVertexArray(0);
	instanceCursor = 0;
}

void SpriteRenderer::DrawSprite(Shader* shader, Texture* texture, glm::vec2 position, glm::vec2 size, glm::vec4 srcRect, float rotate, const glm::vec4& color, int flags)
{
	glm::mat4 orthoProjection = glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);

	shader->Use();
	shader->SetInt("image", 0);
	shader->SetMat4("projection", orthoProjection);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(position, 0));
	// first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
	if ((flags & 4) != 4) model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0)); // move origin of rotation to center of quad
	model = glm::rotate(model, glm::radians(rotate), glm::vec3(0, 0, 1)); // then rotate
	if ((flags & 4) != 4) model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0)); // move origin back
	model = glm::scale(model, glm::vec3(size, 1)); // last scale

	if (srcRect.z != 0)
	{
		srcRect.x /= texture->width;
		srcRect.y /= texture->height;
		srcRect.z /= texture->width;
		srcRect.w /= texture->height;
		srcRect.y = -srcRect.y;
	}

	bool flush = false;
	if (currentShader == nullptr || currentShader->ID != shader->ID)
	{
		if (currentShader != nullptr)
			flush = true;
		else
			currentShader = shader;
	}
	if (currentTexture == nullptr || currentTexture->ID != texture->ID)
	{
		if (currentTexture != nullptr)
			flush = true;
		else
			currentTexture = texture;
	}
	if (instanceCursor >= 200)
		flush = true;

	if (flush)
	{
		Flush();
		currentShader = shader;
		currentTexture = texture;
		instanceCursor = 0; //CA doesn't know Flush() already does this. Whatever.
	}

	//shader.SetMat4("model", model);
	//shader.SetVec4("sourceRect", srcRect);
	//shader.SetVec4("spriteColor", color);
	//shader.SetBool("flipX", (flip & 1) == 1);
	//shader.SetBool("flipY", (flip & 2) == 2);

	models[instanceCursor] = model;
	sourceRects[instanceCursor] = srcRect;
	spriteColors[instanceCursor] = color;
	spriteFlipX[instanceCursor] = ((flags & 1) == 1);
	spriteFlipY[instanceCursor] = ((flags & 2) == 2);
	instanceCursor++;

	//texture.Use(0);

	//glBindVertexArray(this->quadVAO);
	////glDrawArrays(GL_TRIANGLES, 0, 6);
	//glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);
	//glBindVertexArray(0);
}

void SpriteRenderer::DrawSprite(Texture* texture, const glm::vec2& position, const glm::vec2& size, const glm::vec4& srcRect, float rotate, const glm::vec4& color, int flip)
{
	DrawSprite(spriteShader, texture, position, size, srcRect, rotate, color, flip);
}

void SpriteRenderer::DrawSprite(Shader* shader, Texture* texture, const glm::vec2 position)
{
	DrawSprite(shader, texture, position, glm::vec2(texture->width, texture->height), glm::vec4(0), 0, glm::vec4(1));
}

void SpriteRenderer::DrawSprite(Texture* texture, const glm::vec2 position)
{
	DrawSprite(spriteShader, texture, position, glm::vec2(texture->width, texture->height), glm::vec4(0), 0, glm::vec4(1));
}
