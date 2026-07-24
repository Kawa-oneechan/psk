#pragma once
#include <string>
#include <glm/glm.hpp>
#include "Types.h"

namespace Sprite
{
	void DrawText(int font, const std::string& text, glm::vec2 position, const glm::vec4& color = glm::vec4(1), float size = 100, float angle = 0.0f, bool raw = false);
void DrawText(const std::string& text, const glm::vec2& position, const glm::vec4& color = glm::vec4(1), float size = 100, float angle = 0.0f, bool raw = false);
glm::vec2 MeasureText(int font, const std::string& text, float size, bool raw = false);
}

class BeckettFont
{
public:
	virtual ~BeckettFont() = default;
	virtual void Draw(const std::string& text, glm::vec2 position, const glm::vec4& color = glm::vec4(1), float size = 100, float angle = 0.0f, bool raw = false) = 0;
	virtual glm::vec2 Measure(const std::string& text, float size, bool raw = false) = 0;
};

class TrueTypeFont : public BeckettFont
{
private:
	std::string file;
	std::string e0, e1;
	float size;
	float kerning{ 0 };

	void* cdata{ nullptr };
	Texture** fontTextures;

	void loadBank(int bank);

public:
	TrueTypeFont(const std::string& file, float size);
	explicit TrueTypeFont(const jsonValue& json);
	~TrueTypeFont() override;

	virtual void Draw(const std::string& text, glm::vec2 position, const glm::vec4& color = glm::vec4(1), float size = 100, float angle = 0.0f, bool raw = false) override;
	virtual glm::vec2 Measure(const std::string& text, float size, bool raw = false) override;

	TrueTypeFont(const TrueTypeFont&) = delete;
	TrueTypeFont &operator=(const TrueTypeFont&) = delete;

	float lineHeight{ 1.5f };
};

class BitmapFont : public BeckettFont
{
private:
	std::string file;
	float size{ -1 };
	Texture** fontTextures;
	char cdata[0x10000];

	int celWidth{ -1 }, celHeight{ -1 };
	int kerning{ 0 };

	void loadBank(int bank);
	void loadWidths(const jsonObject& json);

public:
	BitmapFont(const std::string& file, int dummy);
	explicit BitmapFont(const jsonValue& json);
	~BitmapFont() override;

	virtual void Draw(const std::string& text, glm::vec2 position, const glm::vec4& color = glm::vec4(1), float size = 100, float angle = 0.0f, bool raw = false) override;
	virtual glm::vec2 Measure(const std::string& text, float size, bool raw = false) override;

	BitmapFont(const BitmapFont&) = delete;
	BitmapFont &operator=(const BitmapFont&) = delete;
};
