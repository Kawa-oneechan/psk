#include <string>
#include <sstream>

#include "SpecialK.h"

#include "SpriteRenderer.h"
#include "support/stb_truetype.h"
#include "support/stb_image_write.h"

extern int width, height;
extern Shader* spriteShader;

namespace UI
{
	extern glm::vec4 primaryColor;
	extern glm::vec4 secondaryColor;
	extern std::vector<glm::vec4> textColors;
	extern JSONObject json;
};

namespace Sprite
{
	constexpr int FontAtlasExtent = 512;
	constexpr int MaxFonts = 4;

	static bool initialized{ false };
	static unsigned int quadVAO{ 0 };
	static glm::vec4 textRenderColor{ 0,0,0,1 }, originalTextRenderColor{ 0,0,0,1 };
	static float textRenderSize{ 100 }, originalTextRenderSize{ 100 };
	static int textRenderFont{ 0 }, originalTextRenderFont{ 0 };

	static Shader* fontShader = nullptr;

	static Texture* currentTexture = nullptr;
	static Shader* currentShader = nullptr;

	static glm::mat4 models[200];
	static glm::vec4 sourceRects[200];
	static glm::vec4 spriteColors[200];
	static int spriteFlipX[200];
	static int spriteFlipY[200];
	static int instanceCursor = 0;

	struct font
	{
		std::string file;
		int size;
		bool alignToGrid;
		int puaSource;
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

		fontShader = new Shader("shaders/font.fs");

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

		glUniformMatrix4fv(glGetUniformLocation(currentShader->ID, "model"), instanceCursor, GL_FALSE, &models[0][0][0]);
		glUniform4fv(glGetUniformLocation(currentShader->ID, "sourceRect"), instanceCursor, &sourceRects[0][0]);
		glUniform4fv(glGetUniformLocation(currentShader->ID, "spriteColor"), instanceCursor, &spriteColors[0][0]);
		glUniform1iv(glGetUniformLocation(currentShader->ID, "flipX"), instanceCursor, &spriteFlipX[0]);
		glUniform1iv(glGetUniformLocation(currentShader->ID, "flipY"), instanceCursor, &spriteFlipY[0]);

		glBindVertexArray(quadVAO);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, instanceCursor);
		glBindVertexArray(0);
		instanceCursor = 0;
	}

	void DrawSprite(Shader* shader, Texture& texture, glm::vec2 position, glm::vec2 size, glm::vec4 srcRect, float rotate, const glm::vec4& color, SpriteFlags flags)
	{
		if (!initialized) Initialize();

		glm::mat4 orthoProjection = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);

		shader->Use();
		shader->Set("image", 0);
		shader->Set("projection", orthoProjection);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(position, 0));
		// first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
		if ((flags & SpriteFlags::TopLeft) != SpriteFlags::TopLeft)
			model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0)); // move origin of rotation to center of quad
		model = glm::rotate(model, glm::radians(rotate), glm::vec3(0, 0, 1)); // then rotate
		if ((flags & SpriteFlags::TopLeft) != SpriteFlags::TopLeft)
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
		if (instanceCursor >= 200)
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

	void DrawSprite(Texture& texture, const glm::vec2& position, const glm::vec2& size, const glm::vec4& srcRect, float rotate, const glm::vec4& color, SpriteFlags flip)
	{
		DrawSprite(spriteShader, texture, position, size, srcRect, rotate, color, flip);
	}

	void DrawSprite(Shader* shader, Texture& texture, const glm::vec2 position)
	{
		DrawSprite(shader, texture, position, glm::vec2(texture.width, texture.height), glm::vec4(0), 0, glm::vec4(1));
	}

	void DrawSprite(Texture& texture, const glm::vec2 position)
	{
		DrawSprite(spriteShader, texture, position, glm::vec2(texture.width, texture.height), glm::vec4(0), 0, glm::vec4(1));
	}

	void DrawLine(const glm::vec2& from, const glm::vec2& to, const glm::vec4& color)
	{
		auto l = to - from;

		auto len = 0.0f;
		{
			auto k = (l.x * l.x) + (l.y * l.y);
			auto n = k / 2;
			auto i = 0x5F3759DF - ((*(int *)&k) >> 1); //what the fuck?
			if (k != 0)
			{
				k = *(float *)&i;
				k = k * (1.5f - (n * k * k));
				len = 1.0f / k;
			}
		}
		auto a = glm::degrees(std::atan2(l.y, l.x));

		DrawSprite(*whiteRect, from, glm::vec2(len, 1), glm::vec4(0), a, color, SpriteFlags::TopLeft);
	}

	static void LoadFontBank(int font, int bank)
	{
		if (!cdata)
		{
			auto doc = VFS::ReadJSON("fonts/fonts.json");
			auto fontSettings = doc->AsArray();
			numFonts = (int)fontSettings.size();
			if (numFonts > MaxFonts)
			{
				numFonts = MaxFonts;
				conprint(2, "Warning: too many font definitions, only doing {}.", MaxFonts);
			}
			cdata = (stbtt_bakedchar*)calloc(numFonts * 0x10000, sizeof(stbtt_bakedchar));
			if (!cdata)
				throw std::runtime_error("Could not allocate space for font atlases.");
			fontTextures = (Texture**)calloc(numFonts * 256, sizeof(Texture*));
			if (!fontTextures)
				throw std::runtime_error("Could not allocate space for font textures.");

			for (int i = 0; i < numFonts; i++)
			{
				auto thisFont = fontSettings[i]->AsObject();
				fonts[i].file = "fonts/" + thisFont["file"]->AsString();
				fonts[i].size = thisFont["size"]->AsInteger();
				fonts[i].alignToGrid = thisFont["grid"] != nullptr ? thisFont["grid"]->AsBool() : false;
				fonts[i].puaSource = thisFont["pua"] != nullptr ? thisFont["pua"]->AsInteger() : 0;
			}
			delete doc;
		}

		if (fontTextures[(font * 256) + bank] != nullptr)
			return;

		auto ttfData = VFS::ReadData(fonts[font].file, nullptr);
		auto ttfBitmap = new unsigned char[FontAtlasExtent * FontAtlasExtent];
		stbtt_BakeFontBitmap((unsigned char*)ttfData.get(), 0, (float)fonts[font].size, ttfBitmap, FontAtlasExtent, FontAtlasExtent, 256 * bank, 256, &cdata[(font * 0xFFFF) + (0x100 * bank)]);

		unsigned int fontID;
		glGenTextures(1, &fontID);
		glBindTexture(GL_TEXTURE_2D, fontID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FontAtlasExtent, FontAtlasExtent, 0, GL_RED, GL_UNSIGNED_BYTE, ttfBitmap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		fontTextures[(font * 256) + bank] = new Texture(fontID, FontAtlasExtent, FontAtlasExtent, 1);
	}

	static void msbtColor(MSBTParams)
	{
		start; len;
		if (tags[0] == "/color")
			textRenderColor = originalTextRenderColor;
		else if (tags.size() < 2)
		{
			//conprint(2, "Missing parameter in MSBT Color");
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

	static void msbtSize(MSBTParams)
	{
		start; len;
		if (tags[0] == "/size")
			textRenderSize = originalTextRenderSize;
		else if (tags.size() < 2)
		{
			//conprint(2, "Missing parameter in MSBT Size");
			return;
		}
		else
		{
			int size = std::stoi(tags[1]);
			if (size == -1)
				textRenderSize = originalTextRenderSize;
			else
				textRenderSize = (float)size;
		}
	}

	static void msbtFont(MSBTParams)
	{
		start; len;
		if (tags[0] == "/font")
			textRenderFont = originalTextRenderFont;
		else if (tags.size() < 2)
		{
			//conprint(2, "Missing parameter in MSBT Font");
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

	typedef void(*MSBTFunc)(MSBTParams);
	const std::map<std::string, MSBTFunc> msbtPhase3
	{
		{ "color", &msbtColor },
		{ "/color", &msbtColor },
		{ "size", &msbtSize },
		{ "/size", &msbtSize },
		{ "font", &msbtFont },
		{ "/font", &msbtFont },
	};

	void DrawText(int font, const std::string& text, glm::vec2 position, const glm::vec4& color, float size, float angle, bool raw)
	{
		if (font >= MaxFonts)
			font = 0;

		if (text.empty())
			return;

		textRenderColor = originalTextRenderColor = color;
		textRenderSize = originalTextRenderSize = size;
		textRenderFont = originalTextRenderFont = font;
		position.y += fonts[font].size * (textRenderSize / 100.0f);

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
			if (bank == 0xE0 && fonts[actualFont].puaSource != 0)
				actualFont = fonts[actualFont].puaSource;
			LoadFontBank(actualFont, bank);

			auto scaleF = textRenderSize / 100.0f;

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
			if (ch == '<' && !raw)
			{
				auto msbtEnd = text.find_first_of('>', i);
				if (msbtEnd == -1) goto renderIt;
				auto msbtStart = i;
				i = msbtEnd + 1;

				auto msbtWhole = text.substr(msbtStart, msbtEnd - msbtStart);
				auto msbt = Split(msbtWhole, ':');
				auto func = msbtPhase3.find(msbt[0]);
				if (func != msbtPhase3.end())
				{
					std::invoke(func->second, msbt, (int)msbtStart - 1, (int)(msbtEnd - msbtStart) + 2);
				}
				continue;
			}

		renderIt:
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

		for (auto& letter : toDraw)
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
		if (font >= MaxFonts)
			font = 0;

		if (text.empty())
			return glm::vec2(0);

		textRenderSize = originalTextRenderSize = size;
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
			if (bank == 0xE0 && fonts[actualFont].puaSource != 0)
				actualFont = fonts[actualFont].puaSource;
			LoadFontBank(actualFont, bank);

			auto scaleF = textRenderSize / 100.0f;

			if (ch == '\n')
			{
				thisLine = 0.0f;
				auto h = cdata[(actualFont * 0xFFFF) + 'A'].x1 - cdata[(actualFont * 0xFFFF) + 'A'].x0;
				result.y += (h + (h / 1)) * scaleF;
				continue;
			}
			if (ch == '<' && !raw)
			{
				auto msbtEnd = text.find_first_of('>', i);
				if (msbtEnd == -1) goto measureIt;
				auto msbtStart = i;
				i = msbtEnd + 1;

				auto msbtWhole = text.substr(msbtStart, msbtEnd - msbtStart);
				auto msbt = Split(msbtWhole, ':');
				if (msbt[0] == "break" || msbt[0] == "clr")
				{
					thisLine = 0.0f;
					result.y = 0;
					continue;
				}
				auto func = msbtPhase3.find(msbt[0]);
				if (func != msbtPhase3.end())
					std::invoke(func->second, msbt, (int)msbtStart - 1, (int)(msbtEnd - msbtStart) + 2);
				continue;
			}

		measureIt:
			auto bakedChar = cdata[(actualFont * 0xFFFF) + ch];

			auto w = bakedChar.x1 - bakedChar.x0 + 0.5f;
			auto h = bakedChar.y1 - bakedChar.y0 + 0.5f;
			auto stringScale = glm::vec2(w * scaleF, h * scaleF);

			thisLine += bakedChar.xadvance * scaleF;

			if (thisLine > result.x)
				result.x = thisLine;
		}

		auto h = cdata[(textRenderFont * 0xFFFF) + 'A'].x1 - cdata[(textRenderFont * 0xFFFF) + 'A'].x0;
		result.y += (h + (h / 2)) * (textRenderSize / 100.0f);

		return result;
	}

}