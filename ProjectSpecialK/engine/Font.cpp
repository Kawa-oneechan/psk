#include <glad/glad.h>
#include <string>
#include <sstream>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

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

__declspec(noreturn)
extern void FatalError(const std::string& message);

static bool initialized{ false };

constexpr int FontBaseScale = 2; //Higher base scale may improve quality.
constexpr int FontAtlasExtent = 512 * FontBaseScale;
constexpr int MaxFonts = 12;

static glm::vec4 textRenderColor{ 0,0,0,1 }, originalTextRenderColor{ 0,0,0,1 };
static float textRenderSize{ 100 }, originalTextRenderSize{ 100 };
static int textRenderFont{ 0 }, originalTextRenderFont{ 0 };

static Shader* fontShader = nullptr;

struct letterToDraw
{
	unsigned int codepoint;
	float angle;
	glm::vec2 scale;
	glm::vec2 position;
	glm::vec4 srcRect;
	glm::vec4 color;
};

static BeckettFont* fonts[MaxFonts];
static int numFonts = 0;

static void LoadFonts()
{
	if (numFonts > 0)
		return;

	auto doc = VFS::ReadJSON("fonts/fonts.json");
	auto fontSettings = doc.as_array();
	numFonts = (int)fontSettings.size();
	if (numFonts > MaxFonts)
	{
		numFonts = MaxFonts;
		conprint(2, "Warning: too many font definitions, only doing {}.", MaxFonts);
	}

	for (int i = 0; i < numFonts; i++)
	{
		auto thisFont = fontSettings[i].as_object();
		auto type = thisFont["type"].as_string();
		if (type == "truetype")
		{
			fonts[i] = new TrueTypeFont(fontSettings[i]);
		}
		else if (type == "bitmap")
		{
			fonts[i] = new BitmapFont(fontSettings[i]);
		}
	}
}

namespace Sprite
{
	void DrawText(int font, const std::string& text, glm::vec2 position, const glm::vec4& color, float size, float angle, bool raw)
	{
		if (numFonts == 0)
			LoadFonts();
		if (text.empty())
			return;
		if (fonts[font] == nullptr)
			return;
		fonts[font]->Draw(text, position, color, size, angle, raw);
	}

	void DrawText(const std::string& text, const glm::vec2& position, const glm::vec4& color, float size, float angle, bool raw)
	{
		DrawText(0, text, position, color, size, angle, raw);
	}

	glm::vec2 MeasureText(int font, const std::string& text, float size, bool raw)
	{
		if (numFonts == 0)
			LoadFonts();
		if (text.empty())
			return glm::vec2(0);
		if (fonts[font] == nullptr)
			return glm::vec2(0);
		return fonts[font]->Measure(text, size, raw);
	}
}


static void Initialize()
{
	if (initialized)
		return;

	fontShader = Shaders["red8"];
	if (fontShader == nullptr)
	{
		fontShader = Shaders["font"];
		if (fontShader == nullptr)
		{
			FatalError("No \"red8\" or \"font\" entry specified in \"shaders/shaders.json\".");
		}
	}

	originalTextRenderSize = textRenderSize = 100;
	originalTextRenderColor = textRenderColor = UI::textColors[0];
	originalTextRenderFont = textRenderFont = 0;

	initialized = true;
}

static void applyRotation(std::vector<letterToDraw>& toDraw, float angle)
{
	if (angle == 0.0f)
		return;

	for (auto& letter : toDraw)
	{
		letter.position = glm::rotate(letter.position, glm::radians(angle));
	}
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

using bjtsFunc = void(*)(BJTSParams);
const std::map<std::string, bjtsFunc> bjtsPhase3
{
	{ "color", &bjtsColor },
	{ "/color", &bjtsColor },
	{ "size", &bjtsSize },
	{ "/size", &bjtsSize },
	{ "font", &bjtsFont },
	{ "/font", &bjtsFont },
};

enum class handleBJTSresult
{
	NoTagHere, //No BJTS tag was found at the current position.
	Continue, //A tag was found and the loop should be continue.
	BreakHere //A break or clr tag was found. Measurement should reset and the loop continued.
};

static handleBJTSresult handleBJTS(const std::string& text, size_t& i, rune ch, bool raw, bool withBreak)
{
	if (ch == '<' && !raw)
	{
		auto bjtsEnd = text.find_first_of('>', i);
		if (bjtsEnd == std::string::npos) return handleBJTSresult::Continue;
		auto bjtsStart = i;
		i = bjtsEnd + 1;

		auto bjtsWhole = text.substr(bjtsStart, bjtsEnd - bjtsStart);
		auto bjts = Split(bjtsWhole, ':');
		if (bjts[0] == "break" || bjts[0] == "clr")
			return handleBJTSresult::BreakHere;
		auto func = bjtsPhase3.find(bjts[0]);
		if (func != bjtsPhase3.end())
		{
			std::invoke(func->second, bjts, (int)bjtsStart - 1, (int)(bjtsEnd - bjtsStart) + 2);
		}
		return handleBJTSresult::Continue;
	}
	return handleBJTSresult::NoTagHere;
}
#endif


TrueTypeFont::TrueTypeFont(const std::string& font, float size)
{
	if (!initialized) Initialize();

	cdata = new stbtt_bakedchar[0x10000]{ 0 };
	fontTextures = new Texture*[256]{ 0 };

	this->file = font;
	this->size = size;
}

TrueTypeFont::TrueTypeFont(const jsonValue& json)
{
	if (!initialized) Initialize();

	cdata = new stbtt_bakedchar[0x10000]{ 0 };
	fontTextures = new Texture*[256]{ 0 };

	auto obj = json.as_object();
	file = obj["file"].as_string();
	size = obj["size"].as_number();
	lineHeight = obj["lineHeight"].is_number() ? obj["lineHeight"].as_number() : lineHeight;
	kerning = obj["kerning"].is_number() ? obj["kerning"].as_number() : 0.0f;
	e0 = obj["e0"].is_string() ? obj["e0"].as_string() : "";
	e1 = obj["e1"].is_string() ? obj["e1"].as_string() : "";
}

TrueTypeFont::~TrueTypeFont()
{
	delete[] cdata;
	delete[] fontTextures;
}

void TrueTypeFont::loadBank(int bank)
{
	if (fontTextures[bank] != nullptr)
		return;

	auto theFile = file;

	if (bank == 0xE0 && !e0.empty())
		theFile = e0;
	else if (bank == 0xE1 && !e1.empty())
		theFile = e1;

	auto ttfData = VFS::ReadData(theFile, nullptr);
	if (!ttfData)
		FatalError(fmt::format("Could not load font {}.", theFile));
	auto ttfBitmap = new unsigned char[FontAtlasExtent * FontAtlasExtent];
	stbtt_BakeFontBitmap(reinterpret_cast<unsigned char*>(ttfData.get()), 0, (float)size  * FontBaseScale, ttfBitmap, FontAtlasExtent, FontAtlasExtent, 256 * bank, 256, &cdata[0x100 * bank]);

	unsigned int fontID;
	glGenTextures(1, &fontID);
	glBindTexture(GL_TEXTURE_2D, fontID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, FontAtlasExtent, FontAtlasExtent, 0, GL_RED, GL_UNSIGNED_BYTE, ttfBitmap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	fontTextures[bank] = new Texture(fontID, FontAtlasExtent, FontAtlasExtent, 1);
}

void TrueTypeFont::Draw(const std::string& text, glm::vec2 position, const glm::vec4& color, float size, float angle, bool raw)
{
	if (text.empty())
		return;

	auto pos = glm::vec2(0);

	textRenderColor = originalTextRenderColor = color;
	//textRenderSize = originalTextRenderSize = size;
	originalTextRenderSize = size;
	textRenderSize = 1.0; //percentage of originalTextRenderSize!
	position.y += this->size * (originalTextRenderSize / 100.0f);

	size_t i = 0;

	std::vector<letterToDraw> toDraw;
	toDraw.reserve(text.length()); //will overshoot on tags and UTF8 but that's cool

	while (text[i] != 0)
	{
		rune ch;
		size_t chs;
		std::tie(ch, chs) = GetChar(text, i);
		i += chs;

		auto bank = ch >> 8;
		loadBank(bank);

		//auto scaleF = textRenderSize / 100.0f;
		auto scaleF = (originalTextRenderSize * textRenderSize) / (100.0f * FontBaseScale);

		if (ch == ' ')
		{
			pos.x += (cdata[' '].xadvance + kerning) * scaleF;
			continue;
		}
		if (ch == '\n')
		{
			pos.x = 0.0f;
			pos.y += (cdata['A'].x1 - cdata['A'].x0) * lineHeight * scaleF;
			continue;
		}

#ifndef BECKETT_NOBJTS
		if (handleBJTS(text, i, ch, raw, false) == handleBJTSresult::Continue)
			continue;
#endif

		auto bakedChar = cdata[ch];

		auto w = bakedChar.x1 - bakedChar.x0 + 0.5f;
		auto h = bakedChar.y1 - bakedChar.y0 + 0.5f;
		auto stringScale = glm::vec2(w * scaleF, h * scaleF);
		auto srcRect = glm::vec4(bakedChar.x0, bakedChar.y0 * -1.0f, w, h * -1.0f);

		auto chPos = pos + glm::vec2(bakedChar.xoff * scaleF, bakedChar.yoff * scaleF);
		toDraw.push_back({ ch, angle, stringScale, chPos, srcRect, textRenderColor });

		pos.x += (bakedChar.xadvance + kerning) * scaleF;
	}

	if (toDraw.empty()) return;

	//Sorting everything in codepoint order should minimize the amount of font texture switching.
	std::sort(toDraw.begin(), toDraw.end(), [](const letterToDraw& a, const letterToDraw& b)
	{
		return (a.codepoint < b.codepoint);
	});

	//TODO: clip

	applyRotation(toDraw, angle);

	for (const auto& letter : toDraw)
	{
		auto bank = letter.codepoint >> 8;
		Sprite::DrawSprite(fontShader, *fontTextures[bank], letter.position + position, letter.scale, letter.srcRect, letter.angle, letter.color, Sprite::SpriteFlags::RotateTopLeft);
	}
}

glm::vec2 TrueTypeFont::Measure(const std::string& text, float size, bool raw)
{
	if (text.empty())
		return glm::vec2(0);

	originalTextRenderSize = size;
	textRenderSize = 1.0;

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
		loadBank(bank);

		//auto scaleF = textRenderSize / 100.0f;
		auto scaleF = (originalTextRenderSize * textRenderSize) / (100.0f * FontBaseScale);

		if (ch == '\n')
		{
			thisLine = 0.0f;
			result.y += cdata['A'].x1 - cdata['A'].x0 * lineHeight * scaleF;
			continue;
		}

#ifndef BECKETT_NOBJTS
		auto bjts = handleBJTS(text, i, ch, raw, true);
		if (bjts == handleBJTSresult::BreakHere)
		{
			thisLine = 0.0f;
			result.y = 0;
			continue;
		}
		else if (bjts == handleBJTSresult::Continue)
			continue;
#endif

		auto bakedChar = cdata[ch];

		auto w = bakedChar.x1 - bakedChar.x0 + 0.5f;
		auto h = bakedChar.y1 - bakedChar.y0 + 0.5f;
		auto stringScale = glm::vec2(w * scaleF, h * scaleF);

		thisLine += (bakedChar.xadvance + kerning) * scaleF;

		if (thisLine > result.x)
			result.x = thisLine;
	}

	auto h = cdata['A'].x1 - cdata['A'].x0;
	result.y += (h * lineHeight) * (originalTextRenderSize / 100.0f);

	return result;
}


BitmapFont::BitmapFont(const std::string& font, int dummy) : file(font)
{
	(void)(dummy);
	if (!initialized) Initialize();

	fontTextures = new Texture*[256]{ 0 };
	cdata[0] = -1;
}

BitmapFont::BitmapFont(const jsonValue& json)
{
	if (!initialized) Initialize();

	if (json.is_string())
	{
		file = json.as_string();
		fontTextures = new Texture*[256]{ 0 };
		cdata[0] = -1;
		return;
	}

	fontTextures = new Texture*[256]{ 0 };

	auto obj = json.as_object();
	file = obj["file"].as_string();
	kerning = obj["kerning"].is_integer() ? obj["kerning"].as_integer() : 0;
	if (!obj["width"].is_null())
	{
		auto w = obj["width"];
		if (w.is_object())
		{
			loadWidths(w.as_object());
		}
		else if (w.is_string())
		{
			loadWidths(VFS::ReadJSON(w.as_string()).as_object());
		}
		else if (w.is_integer())
		{
			std::fill(cdata, cdata + 0xFFFF, w.as_integer());
		}
	}
	else
	{
		cdata[0] = -1;
	}
}


BitmapFont::~BitmapFont()
{
	delete[] fontTextures;
}

void BitmapFont::loadBank(int bank)
{
	if (fontTextures[bank] != nullptr)
		return;

	auto theFile = fmt::format(file, bank);

	fontTextures[bank] = new Texture(theFile, GL_CLAMP, GL_NEAREST, true);
	
	if (celHeight == -1)
	{
		const Texture* tex = fontTextures[bank];
		celWidth = tex->width / 16;
		celHeight = tex->height / 16;
		size = (float)celHeight;
	}

	if (cdata[0] == -1)
	{
		std::fill(cdata, cdata + 0xFFFF, celWidth);
	}
}

void BitmapFont::loadWidths(const jsonObject& json)
{
	for (auto& wl : json)
	{
		auto offset = std::stoi(wl.first, nullptr, 16);
		auto wi = wl.second.as_array();
		for (int i = 0; i < wi.size(); i++)
			cdata[offset + i] = wi[i].as_integer();
	}
}


void BitmapFont::Draw(const std::string& text, glm::vec2 position, const glm::vec4& color, float size, float angle, bool raw)
{
	if (text.empty())
		return;

	auto pos = glm::vec2(0);

	textRenderColor = originalTextRenderColor = color;
	//textRenderSize = originalTextRenderSize = size;
	originalTextRenderSize = size;
	textRenderSize = 2.0; //percentage of originalTextRenderSize!
	position.y += this->size * (originalTextRenderSize / 100.0f);

	size_t i = 0;

	std::vector<letterToDraw> toDraw;
	toDraw.reserve(text.length()); //will overshoot on tags and UTF8 but that's cool

	while (text[i] != 0)
	{
		rune ch;
		size_t chs;
		std::tie(ch, chs) = GetChar(text, i);
		i += chs;

		auto bank = ch >> 8;
		loadBank(bank);

		//auto scaleF = textRenderSize / 100.0f;
		auto scaleF = (originalTextRenderSize * textRenderSize) / (100.0f * FontBaseScale);

		if (ch == ' ')
		{
			pos.x += (cdata[' '] + kerning) * scaleF;
			continue;
		}
		if (ch == '\n')
		{
			pos.x = 0;
			pos.y += celHeight * scaleF;
			continue;
		}

#ifndef BECKETT_NOBJTS
		if (handleBJTS(text, i, ch, raw, false) == handleBJTSresult::Continue)
			continue;
#endif

		auto stringScale = glm::vec2(celWidth * scaleF, celHeight * scaleF);
		auto srcRect = glm::vec4((ch % 16) * celWidth, (ch / 16) * celHeight, celWidth, celHeight);

		auto chPos = pos - glm::vec2(0, celHeight) * scaleF;
		toDraw.push_back({ ch, angle, stringScale, chPos, srcRect, textRenderColor });

		pos.x += (cdata[ch] + kerning) * scaleF;
	}

	if (toDraw.empty()) return;

	//Sorting everything in codepoint order should minimize the amount of font texture switching.
	std::sort(toDraw.begin(), toDraw.end(), [](const letterToDraw& a, const letterToDraw& b)
	{
		return (a.codepoint < b.codepoint);
	});

	//TODO: clip

	applyRotation(toDraw, angle);

	for (const auto& letter : toDraw)
	{
		auto bank = letter.codepoint >> 8;
		Sprite::DrawSprite(*fontTextures[bank], letter.position + position, letter.scale, letter.srcRect, letter.angle, letter.color);
	}
}

glm::vec2 BitmapFont::Measure(const std::string& text, float size, bool raw)
{
	if (text.empty())
		return glm::vec2(0);

	originalTextRenderSize = size;
	textRenderSize = 1.0;

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
		loadBank(bank);

		//auto scaleF = textRenderSize / 100.0f;
		auto scaleF = (originalTextRenderSize * textRenderSize) / (100.0f * FontBaseScale);

		if (ch == '\n')
		{
			thisLine = 0.0f;
			result.y += celHeight * scaleF;
			continue;
		}

#ifndef BECKETT_NOBJTS
		auto bjts = handleBJTS(text, i, ch, raw, true);
		if (bjts == handleBJTSresult::BreakHere)
		{
			thisLine = 0.0f;
			result.y = 0;
			continue;
		}
		else if (bjts == handleBJTSresult::Continue)
			continue;
#endif

		thisLine += cdata[ch] + kerning;

		if (thisLine > result.x)
			result.x = thisLine;
	}

	result.y += celHeight * (originalTextRenderSize / 100.0f);

	return result;
}
