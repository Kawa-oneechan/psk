#include <string>
#include <sstream>

#include "SpriteRenderer.h"
#include "Text.h"
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

static stbtt_bakedchar* cdata;
static Texture** fontTextures;
static std::string fontFiles[MAXFONTS];
static int fontSizes[MAXFONTS];
static int numFonts = 0;

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
		auto& fontSettings = ReadJSON("fonts/fonts.json")->AsArray();
		numFonts = (int)fontSettings.size();
		if (numFonts > MAXFONTS)
		{
			numFonts = MAXFONTS;
			printf("Warning: too many font definitions, only doing %d.\n", MAXFONTS);
		}
		cdata = (stbtt_bakedchar*)calloc(numFonts * 0x10000, sizeof(stbtt_bakedchar));
		if (cdata == nullptr)
			throw std::runtime_error("Could not allocate space for font atlases.");
		fontTextures = (Texture**)calloc(numFonts * 256, sizeof(Texture*));
		if (fontTextures == nullptr)
			throw std::runtime_error("Could not allocate space for font textures.");

		for (int i = 0; i < numFonts; i++)
		{
			auto& thisFont = fontSettings[i]->AsArray();
			fontFiles[i] = "fonts/" + thisFont[0]->AsString();
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

void SpriteRenderer::msbtColor(MSBTParams)
{
	if (tags[0] == "/color")
		textRenderColor = originalTextRenderColor;
	else if (tags.size() < 2)
		return;
	else
	{
		int id = std::stoi(tags[1]);
		if (id == -1 || id >= UI::textColors.size())
			textRenderColor = originalTextRenderColor;
		else
			textRenderColor = UI::textColors[id];
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

void SpriteRenderer::DrawText(int font, const std::string& text, glm::vec2 position, const glm::vec4& color, float size, float angle, bool raw)
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
		if (ch == '<' && !raw)
		{
			auto msbtEnd = text.find_first_of('>', i);
			if (msbtEnd == -1) goto renderIt;
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

renderIt:
		auto chr = ch & 0xFF;

		auto bakedChar = cdata[(textRenderFont * 0xFFFF) + ch];

		auto w = bakedChar.x1 - bakedChar.x0 + 0.5f;
		auto h = bakedChar.y1 - bakedChar.y0 + 0.5f;
		auto stringScale = glm::vec2(w * scaleF, h * scaleF);
		auto srcRect = glm::vec4(bakedChar.x0, bakedChar.y0, w, h);

		auto adv = bakedChar.xadvance;

		auto chPos = position + glm::vec2(bakedChar.xoff * scaleF, bakedChar.yoff * scaleF);
		DrawSprite(fontShader, fontTextures[(textRenderFont * 256) + bank], chPos, stringScale, srcRect, angle, textRenderColor);

		//position.x += adv * scaleF;
		position.x += cosf(glm::radians(angle)) * adv * scaleF;
		position.y += sinf(glm::radians(angle)) * adv * scaleF;
	}
	spriteShader->SetBool("font", false);
}

void SpriteRenderer::DrawText(const std::string& text, const glm::vec2& position, const glm::vec4& color, float size, float angle, bool raw)
{
	DrawText(0, text, position, color, size, angle, raw);
}

glm::vec2 SpriteRenderer::MeasureText(int font, const std::string& text, float size, bool raw)
{
	if (font >= MAXFONTS)
		font = 0;

	textRenderSize = originalTextRenderSize = size;
	textRenderFont = originalTextRenderFont = font;

	glm::vec2 result{ 0 };
	float thisLine = 0.0f;

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

		if (ch == '\n')
		{
			thisLine = 0.0f;
			auto h = cdata[(textRenderFont * 0xFFFF) + 'A'].x1 - cdata[(textRenderFont * 0xFFFF) + 'A'].x0;
			result.y += (h + (h / 2)) * scaleF;
			continue;
		}
		if (ch == '<' && !raw)
		{
			auto msbtEnd = text.find_first_of('>', i);
			if (msbtEnd == -1) goto measureIt;
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

measureIt:
		auto chr = ch & 0xFF;

		auto bakedChar = cdata[(textRenderFont * 0xFFFF) + ch];

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
