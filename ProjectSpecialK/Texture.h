#pragma once

#include "support/glad/glad.h"

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

	Texture() = default;

	Texture(const std::string& texturePath, int repeat = GL_REPEAT, int filter = GL_LINEAR);
	Texture::Texture(const unsigned char* data, int width, int height, int channels, int repeat = GL_REPEAT, int filter = GL_LINEAR);
	Texture(unsigned int id, int width, int height, int channels) : ID(id), width(width), height(height), channels(channels), data(nullptr), filter(GL_LINEAR), repeat(GL_REPEAT) {}

	virtual ~Texture();
	virtual void Use();
	virtual void Use(int slot);

	void SetRepeat(int newRepeat);

	glm::vec4 operator[](size_t i) const { return (atlas.empty() || i >= atlas.size()) ? atlas[0] : atlas[i]; }

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

	TextureArray(const std::string& texturePath, int repeat = GL_REPEAT, int filter = GL_LINEAR);

	virtual ~TextureArray();
	void Use() override;
	void Use(int slot) override;

	void SetRepeat(int newRepeat);

	TextureArray(const TextureArray &x) = default;
	TextureArray &operator=(const TextureArray &x) = default;
};
