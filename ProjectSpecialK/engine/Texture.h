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
	unsigned char* data;
	std::string file;
	int filter, repeat;
	std::vector<glm::vec4> atlas;

public:
	unsigned int ID;
	int width, height, channels;
	bool delayed = false;
	int refCount{ 0 };

	Texture() = default;

	Texture(const std::string& texturePath, int repeat = GL_REPEAT, int filter = 0, bool skipAtlas = false, ColorMap* colorMaps = nullptr, int colorMapIndex = 0);
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
	const size_t Frames() const { return atlas.size(); }

	Texture(const Texture &x) = default;
	Texture &operator=(const Texture &x) = default;
};

class TextureArray : public Texture
{
private:
	unsigned char** data;
	std::string file;
	int filter, repeat;

public:
	int layers;

	TextureArray() = default;
	TextureArray(const std::vector<std::string>& entries, int repeat = GL_REPEAT, int filter = GL_LINEAR);
	TextureArray(const std::string& texturePath, int repeat = GL_REPEAT, int filter = GL_LINEAR);

	virtual ~TextureArray();
	void Use() override;
	void Use(int slot) override;

	TextureArray(const TextureArray &x) = default;
	TextureArray &operator=(const TextureArray &x) = default;
};
