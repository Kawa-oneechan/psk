#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Types.h"

#ifndef GL_REPEAT
#define GL_CLAMP_TO_BORDER 0x812D
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_MIRRORED_REPEAT 0x8370
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_CLAMP 0x2900
#define GL_REPEAT 0x2901
#endif

class Texture
{
private:
	unsigned char* data{ nullptr };
	std::string file;
	int filter{ 0 }, repeat{ 0 };
	SpriteAtlas atlas;

public:
	unsigned int ID{ (unsigned int)-1 };
	int width{ 0 }, height{ 0 }, channels{ 0 };
	bool delayed = false;
	int refCount{ 0 };

	Texture() = default;
	explicit Texture(const std::string& texturePath, int repeat = GL_REPEAT, int filter = 0, bool skipAtlas = false, ColorMap* colorMaps = nullptr, int colorMapIndex = 0);
	Texture(const unsigned char* data, int width, int height, int channels, int repeat = GL_REPEAT, int filter = 0);
	Texture(unsigned int id, int width, int height, int channels) : ID(id), width(width), height(height), channels(channels), data(nullptr), filter(0), repeat(GL_REPEAT) {}

	virtual ~Texture();
	virtual void Use();
	virtual void Use(int slot);

	void SetRepeat(int newRepeat);
	void SetFilter(int newFilter);

	glm::vec4 operator[](size_t i) const
	{
		return (atlas.empty() || i >= atlas.size()) ? atlas[0] : atlas[i];
	}
	glm::vec4 operator[](const std::string& n) const
	{
		return atlas.empty() ? atlas[0] : atlas[n];
	}
	const size_t Frames() const { return atlas.size(); }
	const SpriteAtlas& Atlas() const { return atlas; }
	const void Atlas(const SpriteAtlas& newAtlas) { atlas = newAtlas; }
	const void Atlas(const std::map<std::string, glm::vec4>& newAtlas) { atlas.FromMap(newAtlas); }

	//TODO: look into proper copystructor
	Texture(const Texture &x) = default;
	Texture &operator=(const Texture &x) = delete;
};

class TextureArray : public Texture
{
private:
	unsigned char** data{ nullptr };
	std::string file;
	int filter{ 0 }, repeat{ 0 };

public:
	int layers{ 0 };

	TextureArray() = default;
	explicit TextureArray(const std::vector<std::string>& entries, int repeat = GL_REPEAT, int filter = GL_LINEAR);
	explicit TextureArray(const std::string& texturePath, int repeat = GL_REPEAT, int filter = GL_LINEAR);

	virtual ~TextureArray() override;
	void Use() override;
	void Use(int slot) override;

	//TODO: look into proper copystructor
	TextureArray(const TextureArray &x) = default;
	TextureArray &operator=(const TextureArray &x) = delete;
};
