#include <glad/glad.h>
#include <string>
#include <sstream>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#include <format.h>
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

namespace Sprite
{
	constexpr int FontBaseScale = 2; //Higher base scale may improve quality.
	constexpr int FontAtlasExtent = 512 * FontBaseScale;
	constexpr int MaxFonts = 6;

	static bool initialized{ false };
	static unsigned int quadVAO{ 0 };
	static glm::vec4 textRenderColor{ 0,0,0,1 }, originalTextRenderColor{ 0,0,0,1 };
	static float textRenderSize{ 100 }, originalTextRenderSize{ 100 };
	static int textRenderFont{ 0 }, originalTextRenderFont{ 0 };

	static Shader* fontShader = nullptr;

	static Texture* currentTexture = nullptr;
	static Shader* currentShader = nullptr;

	static glm::mat4 models[BatchSize];
	static glm::vec4 sourceRects[BatchSize];
	static glm::vec4 spriteColors[BatchSize];
	static int spriteFlipX[BatchSize];
	static int spriteFlipY[BatchSize];
	static int instanceCursor = 0;

	struct font
	{
		std::string file;
		int size;
		bool alignToGrid;
		int puaSource1; //Input Prompts
		int puaSource2; //Game-specific
	};

	struct letterToDraw
	{
		unsigned int codepoint;
		int font;
		float angle;
		glm::vec2 scale;
		glm::vec2 position;
		glm::vec4 srcRect;
		glm::vec4 color;
	};

	static stbtt_bakedchar* cdata{ nullptr };
	static Texture** fontTextures;
	static font fonts[MaxFonts];
	static int numFonts = 0;

	static void Initialize()
	{
		if (initialized)
			return;

		unsigned int VBO, EBO;
		float vertices[] = {
			//pos	tex
			0, 1,	0, -1,	//bottom left
			1, 0,	1,  0,	//top right
			0, 0,	0,  0,	//top left
			1, 1,	1, -1,	//bottom right
		};
		int indices[] = {
			0, 1, 2,
			0, 3, 1,
		};

		glGenVertexArrays(1, &quadVAO);
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

		fontShader = Shaders["font"];
		if (fontShader == nullptr)
			FatalError("No \"font\" entry specified in \"shaders/shaders.json\".");

		originalTextRenderSize = textRenderSize = 100;
		originalTextRenderColor = textRenderColor = UI::textColors[0];
		originalTextRenderFont = textRenderFont = 0;

		initialized = true;
	}

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
		glUniform1iv(glGetUniformLocation(currentShader->ID, "flipX"), instanceCursor, &spriteFlipX[0]);
		glUniform1iv(glGetUniformLocation(currentShader->ID, "flipY"), instanceCursor, &spriteFlipY[0]);

		if (currentVAO != quadVAO)
		{
			glBindVertexArray(quadVAO);
			currentVAO = quadVAO;
		}
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, instanceCursor);
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
		spriteFlipX[instanceCursor] = ((flags & FlipX) == FlipX);
		spriteFlipY[instanceCursor] = ((flags & FlipY) == FlipY);
		instanceCursor++;
	}

	void DrawSprite(Texture& texture, const glm::vec2& position, const glm::vec2& size, const glm::vec4& srcRect, float rotate, const glm::vec4& color, SpriteFlags flags)
	{
		if (Shaders["sprite"] == nullptr)
			FatalError("No \"sprite\" entry specified in \"shaders/shaders.json\".");

		DrawSprite(Shaders["sprite"], texture, position, size, srcRect, rotate, color, flags);
	}

	void DrawSprite(Shader* shader, Texture& texture, const glm::vec2& position, const glm::vec4& srcRect, float rotate, const glm::vec4& color, SpriteFlags flags)
	{
		DrawSprite(shader, texture, position, glm::vec2(srcRect.z, srcRect.w), srcRect, rotate, color, flags);
	}

	void DrawSprite(Texture& texture, const glm::vec2& position, const glm::vec4& srcRect, float rotate, const glm::vec4& color, SpriteFlags flags)
	{
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

	static void LoadFontBank(int font, int bank)
	{
		if (!cdata)
		{
			auto doc = VFS::ReadJSON("fonts/fonts.json");
			auto fontSettings = doc.as_array();
			numFonts = (int)fontSettings.size();
			if (numFonts > MaxFonts)
			{
				numFonts = MaxFonts;
				conprint(2, "Warning: too many font definitions, only doing {}.", MaxFonts);
			}
			cdata = new stbtt_bakedchar[numFonts * 0x10000]{ 0 };
			fontTextures = new Texture*[numFonts * 256]{ 0 };

			for (int i = 0; i < numFonts; i++)
			{
				auto thisFont = fontSettings[i].as_object();
				fonts[i].file = "fonts/" + thisFont["file"].as_string();
				fonts[i].size = thisFont["size"].as_integer();
				fonts[i].alignToGrid = thisFont["grid"].is_boolean() ? thisFont["grid"].as_boolean() : false;
				fonts[i].puaSource1 = thisFont["inputs"].is_number() ? thisFont["inputs"].as_integer() : 0;
				fonts[i].puaSource2 = thisFont["icons"].is_number() ? thisFont["icons"].as_integer() : 0;
			}
		}

		if (fontTextures[(font * 256) + bank] != nullptr)
			return;

		auto ttfData = VFS::ReadData(fonts[font].file, nullptr);
		if (!ttfData)
			FatalError(fmt::format("Could not load font {}.", fonts[font].file));
		auto ttfBitmap = new unsigned char[FontAtlasExtent * FontAtlasExtent];
		stbtt_BakeFontBitmap(reinterpret_cast<unsigned char*>(ttfData.get()), 0, (float)fonts[font].size  * FontBaseScale, ttfBitmap, FontAtlasExtent, FontAtlasExtent, 256 * bank, 256, &cdata[(font * 0xFFFF) + (0x100 * bank)]);

		unsigned int fontID;
		glGenTextures(1, &fontID);
		glBindTexture(GL_TEXTURE_2D, fontID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FontAtlasExtent, FontAtlasExtent, 0, GL_RED, GL_UNSIGNED_BYTE, ttfBitmap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		fontTextures[(font * 256) + bank] = new Texture(fontID, FontAtlasExtent, FontAtlasExtent, 1);
	}

#ifndef BECKETT_NOBJTS
	static void bjtsColor(BJTSParams)
	{
		(void)(start); (void)(len);
		if (tags[0] == "/color")
			textRenderColor = originalTextRenderColor;
		else if (tags.size() < 2)
		{
			//conprint(2, "Missing parameter in BJTS Color");
			return;
		}
		else
		{
			int id = std::stoi(tags[1]);
			if (id == -1 || id >= UI::textColors.size())
				textRenderColor = originalTextRenderColor;
			else
				textRenderColor = UI::textColors[id];
		}
	}

	static void bjtsSize(BJTSParams)
	{
		(void)(start); (void)(len);
		if (tags[0] == "/size")
			textRenderSize = 1.0f; //originalTextRenderSize;
		else if (tags.size() < 2)
		{
			//conprint(2, "Missing parameter in BJTS Size");
			return;
		}
		else
		{
			int size = std::stoi(tags[1]);
			if (size == -1)
				textRenderSize = 1.0f; //originalTextRenderSize;
			else
				textRenderSize = (float)size / 100.0f;
		}
	}

	static void bjtsFont(BJTSParams)
	{
		(void)(start); (void)(len);
		if (tags[0] == "/font")
			textRenderFont = originalTextRenderFont;
		else if (tags.size() < 2)
		{
			//conprint(2, "Missing parameter in BJTS Font");
			return;
		}
		else
		{
			int num = std::stoi(tags[1]);
			if (num == -1 || num >= numFonts)
				textRenderFont = originalTextRenderFont;
			else
				textRenderFont = num;
		}
	}

	typedef void(*BJTSFunc)(BJTSParams);
	const std::map<std::string, BJTSFunc> bjtsPhase3
	{
		{ "color", &bjtsColor },
		{ "/color", &bjtsColor },
		{ "size", &bjtsSize },
		{ "/size", &bjtsSize },
		{ "font", &bjtsFont },
		{ "/font", &bjtsFont },
	};
#endif

	void DrawText(int font, const std::string& text, glm::vec2 position, const glm::vec4& color, float size, float angle, bool raw)
	{
		if (numFonts == 0)
			LoadFontBank(0, 0);

		if (font >= MaxFonts)
			font = 0;
		if (font >= numFonts)
			font = numFonts - 1;

		if (text.empty())
			return;

		textRenderColor = originalTextRenderColor = color;
		//textRenderSize = originalTextRenderSize = size;
		originalTextRenderSize = size;
		textRenderSize = 1.0; //percentage of originalTextRenderSize!
		textRenderFont = originalTextRenderFont = font;
		position.y += fonts[font].size * (originalTextRenderSize / 100.0f);

		auto ogX = position.x;
		auto ogY = position.y;
		size_t i = 0;

		std::vector<letterToDraw> toDraw;

		while (text[i] != 0)
		{
			rune ch;
			size_t chs;
			std::tie(ch, chs) = GetChar(text, i);
			i += chs;

			auto bank = ch >> 8;
			auto actualFont = textRenderFont;
			if (bank == 0xE0 && fonts[actualFont].puaSource1 != 0)
				actualFont = fonts[actualFont].puaSource1;
			else if (bank >= 0xE1 && fonts[actualFont].puaSource2 != 0)
				actualFont = fonts[actualFont].puaSource2;
			LoadFontBank(actualFont, bank);

			//auto scaleF = textRenderSize / 100.0f;
			auto scaleF = (originalTextRenderSize * textRenderSize) / (100.0f * FontBaseScale);

			if (ch == ' ')
			{
				auto adv = cdata[(actualFont * 0xFFFF) + ' '].xadvance;
				position.x += cosf(glm::radians(angle)) * adv * scaleF;
				position.y += sinf(glm::radians(angle)) * adv * scaleF;
				continue;
			}
			if (ch == '\n')
			{
				position.x = ogX;
				auto h = cdata[(actualFont * 0xFFFF) + 'A'].x1 - cdata[(actualFont * 0xFFFF) + 'A'].x0;
				ogY += (h + (h / 1)) * scaleF;
				position.y = ogY;
				continue;
			}
#ifndef BECKETT_NOBJTS
			if (ch == '<' && !raw)
			{
				auto bjtsEnd = text.find_first_of('>', i);
				if (bjtsEnd == std::string::npos) goto renderIt;
				auto bjtsStart = i;
				i = bjtsEnd + 1;

				auto bjtsWhole = text.substr(bjtsStart, bjtsEnd - bjtsStart);
				auto bjts = Split(bjtsWhole, ':');
				auto func = bjtsPhase3.find(bjts[0]);
				if (func != bjtsPhase3.end())
				{
					std::invoke(func->second, bjts, (int)bjtsStart - 1, (int)(bjtsEnd - bjtsStart) + 2);
				}
				continue;
			}

		renderIt:
#endif
			auto bakedChar = cdata[(actualFont * 0xFFFF) + ch];

			auto w = bakedChar.x1 - bakedChar.x0 + 0.5f;
			auto h = bakedChar.y1 - bakedChar.y0 + 0.5f;
			auto stringScale = glm::vec2(w * scaleF, h * scaleF);
			auto srcRect = glm::vec4(bakedChar.x0, bakedChar.y0, w, h);

			auto adv = bakedChar.xadvance;

			auto chPos = position + glm::vec2(bakedChar.xoff * scaleF, bakedChar.yoff * scaleF);
			//DrawSprite(fontShader, *fontTextures[(textRenderFont * 256) + bank], chPos, stringScale, srcRect, angle, textRenderColor);
			toDraw.push_back({ ch, actualFont, angle, stringScale, chPos, srcRect, textRenderColor });

			//position.x += adv * scaleF;
			position.x += cosf(glm::radians(angle)) * adv * scaleF;
			position.y += sinf(glm::radians(angle)) * adv * scaleF;
		}

		if (toDraw.empty()) return;

		//Sorting everything in codepoint order should minimize the amount of font texture switching.
		std::sort(toDraw.begin(), toDraw.end(), [](const letterToDraw& a, const letterToDraw& b)
		{
			return (a.codepoint < b.codepoint);
		});

		//TODO: clip

		for (const auto& letter : toDraw)
		{
			auto bank = letter.codepoint >> 8;
			DrawSprite(fontShader, *fontTextures[(letter.font * 256) + bank], letter.position, letter.scale, letter.srcRect, letter.angle, letter.color);
		}
	}

	void DrawText(const std::string& text, const glm::vec2& position, const glm::vec4& color, float size, float angle, bool raw)
	{
		DrawText(0, text, position, color, size, angle, raw);
	}

	glm::vec2 MeasureText(int font, const std::string& text, float size, bool raw)
	{
		if (numFonts == 0)
			LoadFontBank(0, 0);

		if (font >= MaxFonts)
			font = 0;
		if (font >= numFonts)
			font = numFonts - 1;

		if (text.empty())
			return glm::vec2(0);

		//textRenderSize = originalTextRenderSize = size;
		originalTextRenderSize = size;
		textRenderSize = 1.0; //percentage!
		textRenderFont = originalTextRenderFont = font;

		glm::vec2 result{ 0 };
		float thisLine = 0.0f;

		size_t i = 0;

		while (text[i] != 0)
		{
			rune ch;
			size_t chs;
			std::tie(ch, chs) = GetChar(text, i);
			i += chs;

			auto bank = ch >> 8;
			auto actualFont = textRenderFont;
			if (bank == 0xE0 && fonts[actualFont].puaSource1 != 0)
				actualFont = fonts[actualFont].puaSource1;
			else if (bank >= 0xE1 && fonts[actualFont].puaSource2 != 0)
				actualFont = fonts[actualFont].puaSource2;
			LoadFontBank(actualFont, bank);

			//auto scaleF = textRenderSize / 100.0f;
			auto scaleF = (originalTextRenderSize * textRenderSize) / (100.0f * FontBaseScale);

			if (ch == '\n')
			{
				thisLine = 0.0f;
				auto h = cdata[(actualFont * 0xFFFF) + 'A'].x1 - cdata[(actualFont * 0xFFFF) + 'A'].x0;
				result.y += (h + (h / 1)) * scaleF;
				continue;
			}
#ifndef BECKETT_NOBJTS
			if (ch == '<' && !raw)
			{
				auto bjtsEnd = text.find_first_of('>', i);
				if (bjtsEnd == std::string::npos) goto measureIt;
				auto bjtsStart = i;
				i = bjtsEnd + 1;

				auto bjtsWhole = text.substr(bjtsStart, bjtsEnd - bjtsStart);
				auto bjts = Split(bjtsWhole, ':');
				if (bjts[0] == "break" || bjts[0] == "clr")
				{
					thisLine = 0.0f;
					result.y = 0;
					continue;
				}
				auto func = bjtsPhase3.find(bjts[0]);
				if (func != bjtsPhase3.end())
					std::invoke(func->second, bjts, (int)bjtsStart - 1, (int)(bjtsEnd - bjtsStart) + 2);
				continue;
			}

		measureIt:
#endif
			auto bakedChar = cdata[(actualFont * 0xFFFF) + ch];

			auto w = bakedChar.x1 - bakedChar.x0 + 0.5f;
			auto h = bakedChar.y1 - bakedChar.y0 + 0.5f;
			auto stringScale = glm::vec2(w * scaleF, h * scaleF);

			thisLine += bakedChar.xadvance * scaleF;

			if (thisLine > result.x)
				result.x = thisLine;
		}

		auto h = cdata[(textRenderFont * 0xFFFF) + 'A'].x1 - cdata[(textRenderFont * 0xFFFF) + 'A'].x0;
		result.y += (h + (h / 2)) * (originalTextRenderSize / 100.0f);

		return result;
	}

}
