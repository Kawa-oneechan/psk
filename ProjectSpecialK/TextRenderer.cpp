#include <string>
#include <sstream>

#include "SpecialK.h"
#include "support/stb_truetype.h"
#include "support/stb_image_write.h"

extern Shader* spriteShader;
extern float width, height;
namespace UI
{
	extern glm::vec4 primaryColor;
	extern glm::vec4 secondaryColor;
	extern std::vector<glm::vec4> textColors;
	extern JSONObject json;
};

typedef struct
{
	std::string file;
	int size;
	bool alignToGrid;
	int puaSource;
} font;

static stbtt_bakedchar* cdata{ nullptr };
static Texture** fontTextures;
static font fonts[MaxFonts];
static int numFonts = 0;

typedef struct
{
	unsigned int codepoint;
	int font;
	float angle;
	glm::vec2 scale;
	glm::vec2 position;
	glm::vec4 srcRect;
	glm::vec4 color;
} letterToDraw;

static void FlipImage(unsigned char* image, int width, int height)
{
	int row;
	unsigned char temp[FontAtlasExtent * 4];
	unsigned char* bytes = (unsigned char*)image;

	for (row = 0; row < (height >> 1); row++)
	{
		unsigned char *row0 = bytes + row * width;
		unsigned char *row1 = bytes + (width - row - 1) * width;
		//TODO: improve this -- is there some C++17 algo bullshit for this?
		memcpy(temp, row0, width);
		memcpy(row0, row1, width);
		memcpy(row1, temp, width);
		row0 += width;
		row1 += width;
	}
}

void SpriteRenderer::LoadFontBank(int font, int bank)
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
	FlipImage(ttfBitmap, FontAtlasExtent, FontAtlasExtent);

	unsigned int fontID;
	glGenTextures(1, &fontID);
	glBindTexture(GL_TEXTURE_2D, fontID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FontAtlasExtent, FontAtlasExtent, 0, GL_RED, GL_UNSIGNED_BYTE, ttfBitmap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	fontTextures[(font * 256) + bank] = new Texture(fontID, FontAtlasExtent, FontAtlasExtent, 1);
}

void SpriteRenderer::msbtColor(MSBTParams)
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

void SpriteRenderer::msbtSize(MSBTParams)
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

void SpriteRenderer::msbtFont(MSBTParams)
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

void SpriteRenderer::DrawText(int font, const std::string& text, glm::vec2 position, const glm::vec4& color, float size, float angle, bool raw)
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
				std::invoke(func->second, this, msbt, (int)msbtStart - 1, (int)(msbtEnd - msbtStart) + 2);
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

void SpriteRenderer::DrawText(const std::string& text, const glm::vec2& position, const glm::vec4& color, float size, float angle, bool raw)
{
	DrawText(0, text, position, color, size, angle, raw);
}

glm::vec2 SpriteRenderer::MeasureText(int font, const std::string& text, float size, bool raw)
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
				std::invoke(func->second, this, msbt, (int)msbtStart - 1, (int)(msbtEnd - msbtStart) + 2);
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
