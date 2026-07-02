#include <glad/glad.h>
#include <string>
#include <sstream>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

#include <fmt/format.h>
#include <stb_truetype.h>
#include <stb_image_write.h>
#include "SpriteRenderer.h"
#include "TextUtils.h"
#include "Console.h"
#include "Types.h"
#include "Shader.h"
#include "Texture.h"
#include "VFS.h"

extern int width, height;
extern Texture* whiteRect;

unsigned int currentVAO = 0;

__declspec(noreturn)
	extern void FatalError(const std::string& message);

static bool initialized{ false };
static unsigned int quadVAO{ 0 };

static Texture* currentTexture = nullptr;
static Shader* currentShader = nullptr;

static glm::mat4 models[Sprite::BatchSize];
static glm::vec4 sourceRects[Sprite::BatchSize];
static glm::vec4 spriteColors[Sprite::BatchSize];
static int instanceCursor = 0;

static void Initialize()
{
	if (initialized)
		return;

	glGenVertexArrays(1, &quadVAO);

#ifndef BECKETT_PULLEDPORK
	unsigned int VBO, EBO;
	float vertices[] = {
		//pos	tex
		0, 0,	0,  0,	//top left
		0, 1,	0, -1,	//bottom left
		1, 0,	1,  0,	//top right
		1, 1,	1, -1,	//bottom right
	};
	int indices[] = {
		2, 1, 0,
		2, 3, 1,
	};

	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(quadVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
#else
	//If we're pulling, quadVAO is merely a placeholder.
#endif

	initialized = true;
}

namespace Sprite
{
	void FlushBatch()
	{
		if (instanceCursor == 0)
			return;

		currentShader->Use();
		currentTexture->Use(0);

		//this is bullshit
		int vp[4];
		glGetIntegerv(GL_VIEWPORT, vp);

		glm::mat4 orthoProjection = glm::ortho((float)vp[0], (float)vp[2], (float)vp[3], (float)vp[1], -1.0f, 1.0f);
		currentShader->Use();
		currentShader->Set("image", 0);
		currentShader->Set("projection", orthoProjection);

		glUniformMatrix4fv(glGetUniformLocation(currentShader->ID, "model"), instanceCursor, GL_FALSE, &models[0][0][0]);
		glUniform4fv(glGetUniformLocation(currentShader->ID, "sourceRect"), instanceCursor, &sourceRects[0][0]);
		glUniform4fv(glGetUniformLocation(currentShader->ID, "spriteColor"), instanceCursor, &spriteColors[0][0]);

		if (currentVAO != quadVAO)
		{
			glBindVertexArray(quadVAO);
			currentVAO = quadVAO;
		}
		glDisable(GL_CULL_FACE); //so flipped sprites still show up
#ifndef BECKETT_PULLEDPORK
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, instanceCursor);
#else
		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, instanceCursor);
#endif
		glEnable(GL_CULL_FACE);
		instanceCursor = 0;
	}

	void DrawSprite(Shader* shader, Texture& texture, glm::vec2 position, glm::vec2 size, glm::vec4 srcRect, float rotate, const glm::vec4& color, SpriteFlags flags)
	{
		if (!initialized) Initialize();

		if ((flags & SpriteFlags::MidBotOrigin) == SpriteFlags::MidBotOrigin)
			position -= glm::vec2(size.x * 0.5f, size.y);
		else if ((flags & SpriteFlags::CenterOrigin) == SpriteFlags::CenterOrigin)
			position -= size * 0.5f;

		//Clip any sprites that aren't visible
		if ((flags & SpriteFlags::NoClip) != SpriteFlags::NoClip)
		{
			int vp[4]; glGetIntegerv(GL_VIEWPORT, vp);
			if ((int)position.x > vp[2] ||
				(int)position.x + size.x < vp[0] ||
				(int)position.y > vp[3] ||
				(int)position.y + size.y < vp[1])
				return;
		}
		
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(position, 0));
		// first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)

		if ((flags & SpriteFlags::FlipX) == SpriteFlags::FlipX)
		{
			model = glm::translate(model, glm::vec3(size.x, 0, 0));
			model = glm::scale(model, glm::vec3(-1, 1, 1));
		}
		if ((flags & SpriteFlags::FlipY) == SpriteFlags::FlipY)
		{
			model = glm::translate(model, glm::vec3(0, size.y, 0));
			model = glm::scale(model, glm::vec3(1, -1, 1));
		}

		if ((flags & SpriteFlags::RotateTopLeft) != SpriteFlags::RotateTopLeft)
			model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0)); // move origin of rotation to center of quad
		model = glm::rotate(model, glm::radians(rotate), glm::vec3(0, 0, 1)); // then rotate
		if ((flags & SpriteFlags::RotateTopLeft) != SpriteFlags::RotateTopLeft)
			model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0)); // move origin back
		model = glm::scale(model, glm::vec3(size, 1)); // last scale

		if (srcRect.z != 0)
		{
			srcRect.x /= texture.width;
			srcRect.y /= texture.height;
			srcRect.z /= texture.width;
			srcRect.w /= texture.height;
			srcRect.y = -srcRect.y;
		}

		bool flush = false;
		if (!currentShader || currentShader->ID != shader->ID)
		{
			if (currentShader != nullptr)
				flush = true;
			else
				currentShader = shader;
		}
		if (!currentTexture || currentTexture->ID != texture.ID)
		{
			if (currentTexture != nullptr)
				flush = true;
			else
				currentTexture = &texture;
		}
		if (instanceCursor >= BatchSize)
			flush = true;

		if (flush)
		{
			FlushBatch();
			currentShader = shader;
			currentTexture = &texture;
			instanceCursor = 0; //CA doesn't know Flush() already does this. Whatever.
		}

		models[instanceCursor] = model;
		sourceRects[instanceCursor] = srcRect;
		spriteColors[instanceCursor] = color;
		instanceCursor++;
	}

	void DrawSprite(Texture& texture, const glm::vec2& position, const glm::vec2& size, const glm::vec4& srcRect, float rotate, const glm::vec4& color, SpriteFlags flags)
	{
		if (Shaders["sprite"] == nullptr)
			FatalError("No \"sprite\" entry specified in \"shaders/shaders.json\".");

		if (texture.channels == 1)
			DrawSprite(Shaders["red8"], texture, position, size, srcRect, rotate, color, flags);
		else
			DrawSprite(Shaders["sprite"], texture, position, size, srcRect, rotate, color, flags);
	}

	void DrawSprite(Shader* shader, Texture& texture, const glm::vec2& position, const glm::vec4& srcRect, float rotate, const glm::vec4& color, SpriteFlags flags)
	{
		DrawSprite(shader, texture, position, glm::vec2(srcRect.z, srcRect.w), srcRect, rotate, color, flags);
	}

	void DrawSprite(Texture& texture, const glm::vec2& position, const glm::vec4& srcRect, float rotate, const glm::vec4& color, SpriteFlags flags)
	{
		if (texture.channels == 1)
			DrawSprite(Shaders["red8"], texture, position, glm::vec2(srcRect.z, srcRect.w), srcRect, rotate, color, flags);
		else
			DrawSprite(Shaders["sprite"], texture, position, glm::vec2(srcRect.z, srcRect.w), srcRect, rotate, color, flags);
	}

	void DrawSprite(Shader* shader, Texture& texture, const glm::vec2 position)
	{
		DrawSprite(shader, texture, position, glm::vec2(texture.width, texture.height), glm::vec4(0), 0, glm::vec4(1));
	}

	void DrawSprite(Texture& texture, const glm::vec2 position)
	{
		DrawSprite(Shaders["sprite"], texture, position, glm::vec2(texture.width, texture.height), glm::vec4(0), 0, glm::vec4(1));
	}

	void DrawLine(const glm::vec2& from, const glm::vec2& to, const glm::vec4& color)
	{
		auto l = to - from;
		auto a = glm::degrees(std::atan2(l.y, l.x));

		auto k = (l.x * l.x) + (l.y * l.y);
		__m128 temp = _mm_set_ss(k);
		temp = _mm_rsqrt_ss(temp);
		auto len = 1.0f / _mm_cvtss_f32(temp);

		DrawSprite(*whiteRect, from, glm::vec2(len, 1), glm::vec4(0), a, color, (SpriteFlags)(SpriteFlags::RotateTopLeft | SpriteFlags::NoClip));
	}

	void DrawRect(const glm::vec4& bounds, const glm::vec4& color)
	{
		auto h = glm::vec2(bounds.z - bounds.x, 1);
		auto v = glm::vec2(1, bounds.w - bounds.y + 1);
		DrawSprite(*whiteRect, glm::vec2(bounds.x, bounds.y), h, glm::vec4(0), 0.0f, color);
		DrawSprite(*whiteRect, glm::vec2(bounds.x, bounds.w), h, glm::vec4(0), 0.0f, color);
		DrawSprite(*whiteRect, glm::vec2(bounds.x, bounds.y), v, glm::vec4(0), 0.0f, color);
		DrawSprite(*whiteRect, glm::vec2(bounds.z, bounds.y), v, glm::vec4(0), 0.0f, color);
	}

	void DrawCircle(const glm::vec2& center, float radius, int segments, const glm::vec4& color)
	{
		auto step = glm::tau<float>() / segments;
		auto there = 0.0f;
		auto here = step;
		for (int i = 0; i < segments; i++)
		{
			auto from = center + glm::vec2(glm::sin(there) * radius, glm::cos(there) * radius);
			auto to   = center + glm::vec2(glm::sin( here) * radius, glm::cos( here) * radius);
			DrawLine(from, to, color);
			there = here;
			here += step;
		}
	}

	void DrawPoly(const polygon& poly, const glm::vec2& origin, float scale, const glm::vec4& color)
	{
		for (int i = 1; i < poly.size(); i++)
			DrawLine(origin + (poly[i - 1] * scale), origin + (poly[i] * scale), color);
		DrawLine(origin + (poly[poly.size() - 1] * scale), origin + (poly[0] * scale), color);
	}
}
