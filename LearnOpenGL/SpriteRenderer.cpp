#include <string>
#include <sstream>

#include "SpriteRenderer.h"
#include "support/stb_truetype.h"
#include "support/stb_image_write.h"

extern Shader* spriteShader;

extern float width, height;

static stbtt_bakedchar* cdata;
static Texture** fontTextures;
static std::string fontFiles[MAXFONTS];
static int fontSizes[MAXFONTS];
static int numFonts = 0;

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

	fontShader = new Shader("font.fs");

	originalTextRenderSize = textRenderSize = 100;
	originalTextRenderColor = textRenderColor = glm::vec4(0, 0, 0, 1);
	originalTextRenderFont = textRenderFont = 0;
}

SpriteRenderer::~SpriteRenderer()
{
	glDeleteVertexArrays(1, &this->quadVAO);
}

void SpriteRenderer::DrawSprite(Shader& shader, Texture &texture, glm::vec2 position, glm::vec2 size, glm::vec4 srcRect, float rotate, const glm::vec4& color, int flip)
{
	glm::mat4 orthoProjection = glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);

	shader.Use();
	shader.SetInt("image", 0);
	shader.SetMat4("projection", orthoProjection);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(position, 0));
	// first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
	model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0)); // move origin of rotation to center of quad
	model = glm::rotate(model, glm::radians(rotate), glm::vec3(0, 0, 1)); // then rotate
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

	shader.SetMat4("model", model);
	shader.SetVec4("sourceRect", srcRect);
	shader.SetVec4("spriteColor", color);
	shader.SetBool("flipX", (flip & 1) == 1);
	shader.SetBool("flipY", (flip & 2) == 2);

	texture.Use(0);


	glBindVertexArray(this->quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void SpriteRenderer::DrawSprite(Texture &texture, const glm::vec2& position, const glm::vec2& size, const glm::vec4& srcRect, float rotate, const glm::vec4& color, int flip)
{
	DrawSprite(*spriteShader, texture, position, size, srcRect, rotate, color, flip);
}

void SpriteRenderer::DrawSprite(Shader &shader, Texture &texture, const glm::vec2 position)
{
	DrawSprite(shader, texture, position, glm::vec2(texture.width, texture.height), glm::vec4(0), 0, glm::vec4(1));
}

void SpriteRenderer::DrawSprite(Texture & texture, const glm::vec2 position)
{
	DrawSprite(*spriteShader, texture, position, glm::vec2(texture.width, texture.height), glm::vec4(0), 0, glm::vec4(1));
}

static void FlipImage(unsigned char* image, int width, int height)
{
	int row;
	unsigned char temp[2048];
	unsigned char* bytes = (unsigned char*)image;

	for (row = 0; row < (height >> 1); row++)
	{
		unsigned char *row0 = bytes + row * width;
		unsigned char *row1 = bytes + (width - row - 1) * width;
		// swap row0 with row1
		memcpy(temp, row0, width);
		memcpy(row0, row1, width);
		memcpy(row1, temp, width);
		row0 += width;
		row1 += width;
	}
}

void SpriteRenderer::LoadFontBank(int font, int bank)
{
	if (cdata == nullptr)
	{
		auto& fontSettings = ReadJSON("fonts.json")->AsArray();
		numFonts = (int)fontSettings.size();
		if (numFonts > MAXFONTS)
		{
			numFonts = MAXFONTS;
			printf("Warning: too many font definitions, only doing %d.\n", MAXFONTS);
		}
		cdata = (stbtt_bakedchar*)calloc(numFonts * 0x10000, sizeof(stbtt_bakedchar));
		fontTextures = (Texture**)calloc(numFonts * 256, sizeof(Texture*));
		if (fontTextures == nullptr)
			throw std::runtime_error("Could not allocate space for font textures.");

		for (int i = 0; i < numFonts; i++)
		{
			auto& thisFont = fontSettings[i]->AsArray();
			fontFiles[i] = thisFont[0]->AsString();
			fontSizes[i] = (int)thisFont[1]->AsNumber();
		}
		printf("fontSettings!");
	}

	if (fontTextures[(font * 256) + bank] != nullptr)
		return;

//	if (fontTextures[font][bank] != nullptr)
//		return;

	auto ttfData = (unsigned char*)ReadVFS(fontFiles[font], nullptr);
	auto ttfBitmap = (unsigned char*)malloc(FONTATLAS_SIZE * FONTATLAS_SIZE);
	stbtt_BakeFontBitmap(ttfData, 0, (float)fontSizes[font], ttfBitmap, FONTATLAS_SIZE, FONTATLAS_SIZE, 256 * bank, 256, &cdata[(font * 0xFFFF) + (0x100 * bank)]);
	FlipImage(ttfBitmap, FONTATLAS_SIZE, FONTATLAS_SIZE);

	unsigned int fontID;
	glGenTextures(1, &fontID);
	glBindTexture(GL_TEXTURE_2D, fontID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FONTATLAS_SIZE, FONTATLAS_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, ttfBitmap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	free(ttfBitmap);

	//Texture font(fontID, FONTATLAS_SIZE, FONTATLAS_SIZE, 1);
	fontTextures[(font * 256) + bank] = new Texture(fontID, FONTATLAS_SIZE, FONTATLAS_SIZE, 1);
}

//TEMPORARY -- is in Text.cpp in real PSK
std::vector<std::string> Split(std::string& data, char delimiter)
{
	std::vector<std::string> ret;
	std::string part;
	std::istringstream stream(data);
	while (std::getline(stream, part, delimiter))
		ret.push_back(part);
	return ret;
}

static const std::vector<glm::vec4> textColors
{
	glm::vec4(0.961f, 0.506f, 0.773f, 1.0f), //#FFF581C5 Town
	glm::vec4(0.024f, 0.714f, 0.820f, 1.0f), //#FF06B6D1 Important
	glm::vec4(0.000f, 0.761f, 0.000f, 1.0f), //#FF00C200 Player
	glm::vec4(0.941f, 0.549f, 0.000f, 1.0f), //#FFF08C00 NPC
	glm::vec4(0.510f, 0.443f, 0.341f, 0.5f), //#80827157 Onomatopoeia
	glm::vec4(0.847f, 0.247f, 0.247f, 1.0f), //#FFD83F3F CampGameRed
	glm::vec4(0.365f, 0.365f, 0.400f, 1.0f), //#FF5D5D66 CampGameBlack
};

void SpriteRenderer::msbtColor(MSBTParams)
{
	if (tags[0] == "/color")
		textRenderColor = originalTextRenderColor;
	else
	{
		int id = std::stoi(tags[1]);
		if (id == -1 || id >= textColors.size())
			textRenderColor = originalTextRenderColor;
		else
			textRenderColor = textColors[id];
	}
}

void SpriteRenderer::msbtSize(MSBTParams)
{
	if (tags[0] == "/size")
		textRenderSize = originalTextRenderSize;
	else
	{
		int size = std::stoi(tags[1]);
		if (size == -1)
			textRenderSize = originalTextRenderSize;
		else
			textRenderSize = (float)size;
	}
}

void SpriteRenderer::msbtFont(MSBTParams)
{
	if (tags[0] == "/font")
		textRenderFont = originalTextRenderFont;
	else
	{
		int num = std::stoi(tags[1]);
		if (num == -1 || num >= numFonts)
			textRenderFont = originalTextRenderFont;
		else
			textRenderFont = num;
	}
}

void SpriteRenderer::DrawText(int font, const std::string& text, glm::vec2 position, const glm::vec4& color, float size, float angle)
{
	if (font >= MAXFONTS)
		font = 0;

	textRenderColor = originalTextRenderColor = color;
	textRenderSize = originalTextRenderSize = size;
	textRenderFont = originalTextRenderFont = font;
	position.y += fontSizes[font] * (textRenderSize / 100.0f);

	auto ogX = position.x;
	auto ogY = position.y;
	spriteShader->SetBool("font", true);
	size_t i = 0;

	while (text[i] != 0)
	{
		//UTF-8 decoder woo
		unsigned int ch = text[i++] & 0xFF;
		if ((ch & 0xE0) == 0xC0)
		{
			ch = (ch & 0x1F) << 6;
			ch |= (text[i++] & 0x3F);
		}
		else if ((ch & 0xF0) == 0xE0)
		{
			ch = (ch & 0x1F) << 12;
			ch |= (text[i++] & 0x3F) << 6;
			ch |= (text[i++] & 0x3F);
		}

		auto bank = ch >> 8;
		LoadFontBank(textRenderFont, bank);

		auto scaleF = textRenderSize / 100.0f;

		if (ch == ' ')
		{
			auto adv = cdata[(textRenderFont * 0xFFFF) + ' '].xadvance;
			position.x += cosf(glm::radians(angle)) * adv * scaleF;
			position.y += sinf(glm::radians(angle)) * adv * scaleF;
			continue;
		}
		if (ch == '\n')
		{
			position.x = ogX;
			auto h = cdata[(textRenderFont * 0xFFFF) + 'A'].x1 - cdata[(textRenderFont * 0xFFFF) + 'A'].x0;
			ogY += (h + (h / 2)) * scaleF;
			position.y = ogY;
			continue;
		}
		if (ch == '<')
		{
			auto msbtEnd = text.find_first_of('>', i);
			auto msbtStart = i;
			i = msbtEnd + 1;

			auto msbtWhole = text.substr(msbtStart, msbtEnd - msbtStart);
			//fmt::print("(MSBT: {})", msbtWhole);
			auto msbt = Split(msbtWhole, ':');
			auto func = msbtPhase3.find(msbt[0]);
			if (func != msbtPhase3.end())
			{
				std::invoke(func->second, this, msbt, (int)msbtStart - 1, (int)(msbtEnd - msbtStart) + 2);
				//func->second(msbt, (int)msbtStart - 1, (int)(msbtEnd - msbtStart) + 2);
			}
			continue;
		}

		auto chr = ch & 0xFF;

		auto bakedChar = cdata[(textRenderFont * 0xFFFF) + ch];

		auto w = bakedChar.x1 - bakedChar.x0 + 0.5f;
		auto h = bakedChar.y1 - bakedChar.y0 + 0.5f;
		auto stringScale = glm::vec2(w * scaleF, h * scaleF);
		auto srcRect = glm::vec4(bakedChar.x0, bakedChar.y0, w, h);

		auto adv = bakedChar.xadvance;

		auto chPos = position + glm::vec2(bakedChar.xoff * scaleF, bakedChar.yoff * scaleF);
		DrawSprite(*fontShader, *fontTextures[(textRenderFont * 256) + bank], chPos, stringScale, srcRect, angle, textRenderColor);

		//position.x += adv * scaleF;
		position.x += cosf(glm::radians(angle)) * adv * scaleF;
		position.y += sinf(glm::radians(angle)) * adv * scaleF;
	}
	spriteShader->SetBool("font", false);
}

void SpriteRenderer::DrawText(const std::string& text, const glm::vec2& position, const glm::vec4& color, float size, float angle)
{
	DrawText(0, text, position, color, size, angle);
}
