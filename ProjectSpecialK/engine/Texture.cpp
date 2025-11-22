#include <glad/glad.h>
#include <stb_image.h>
#include "JsonUtils.h"
#include "TextUtils.h"
#include "Texture.h"
#include "VFS.h"
#include "Console.h"
#include "../Game.h"

#ifndef BECKETT_DEFAULTFILTER
#define BECKETT_DEFAULTFILTER GL_LINEAR
#endif

static std::map<std::string, Texture*> cache;
static std::map<std::string, TextureArray*> cacheArray;

static unsigned int currentTextureSlot = 0;
static unsigned int currentTexture[32]{ 0 };

static bool load(const unsigned char* data, unsigned int *id, int width, int height, int channels, int repeat, int filter)
{
	glGenTextures(1, id);
	if (glGetError())
		return false;

	int format = (channels == 4) ? GL_RGBA : ((channels == 1) ? GL_RED : GL_RGB);
	int target = (channels >= 3) ? GL_RGBA8 : GL_RED;

	glBindTexture(GL_TEXTURE_2D, *id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

	glTexImage2D(GL_TEXTURE_2D, 0, target, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

Texture::Texture(const std::string& texturePath, int repeat, int filter, bool skipAtlas, ColorMap* colors, int colorIndex) : file(texturePath), repeat(repeat)
{
	ID = 0;
	width = height = channels = 0;
	data = nullptr;

	this->filter = filter == 0 ? BECKETT_DEFAULTFILTER : filter;

	auto c = cache.find(texturePath);
	if (c != cache.end())
	{
		Texture* t = c->second;
		ID = t->ID;
		width = t->width;
		height = t->height;
		channels = t->channels;
		data = t->data;
		delayed = t->delayed;
		this->repeat = t->repeat;
		this->filter = t->filter;
		atlas = t->atlas;
		t->refCount++;
		refCount = t->refCount;
		return;
	}

	refCount = 1;
	stbi_set_flip_vertically_on_load(1);

	size_t vfsSize = 0;
	auto vfsData = VFS::ReadData(texturePath, &vfsSize);
	if (!vfsData || vfsSize == 0)
	{
		conprint(1, "Failed to load texture \"{}\" -- no data.", texturePath);
		return;
	}
	data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(vfsData.get()), (int)vfsSize, &width, &height, &channels, 0);

	if (!skipAtlas)
	{
		GetAtlas(atlas, VFS::ClimbDown(VFS::ChangeExtension(texturePath, "json"), "atlas.json"));
	}
	if (atlas.empty())
		atlas.push_back(glm::vec4(0, 0, width, height));

	if (data)
	{
		if (colors && colorIndex > 0 && colorIndex < colors->numRows)
		{
			auto d = reinterpret_cast<unsigned int*>(data);
			for (int i = 0; i < width * height; i++)
			{
				for (int key = 0; key < colors->numCols; key++)
				{
					if (d[i] == colors->values[key])
						d[i] = colors->values[(colorIndex * ColorMap::Cols) + key];
				}
			}
		}

		if (!load(data, &ID, width, height, channels, repeat, this->filter))
		{
			debprint(3, "glGenTextures indicates we're threading. Delaying \"{}\"...", texturePath);
			delayed = true;
			cache[file] = this;
			return;
		}
	}
	else
	{
		conprint(1, "Failed to load texture \"{}\" -- invalid data.", texturePath);
	}
	stbi_image_free(data);

	cache[file] = this;
}

Texture::Texture(const unsigned char* externalData, int width, int height, int channels, int repeat, int filter) : data(nullptr), width(width), height(height), channels(channels), repeat(repeat)
{
	ID = 0;
	this->file.clear();

	this->filter = filter == 0 ? BECKETT_DEFAULTFILTER : filter;

	atlas.push_back(glm::vec4(0, 0, width, height));

	//int format = GL_RGB;
	//if (channels == 4) format = GL_RGBA;

	if (externalData)
	{
		if (!load(externalData, &ID, width, height, channels, repeat, this->filter))
		{
			debprint(3, "glGenTextures indicates we're threading. Delaying load from memory...");
			delayed = true;
			//grab a copy we control for later
			auto size = width * height * channels;
			this->data = new unsigned char[size];
			std::memcpy(this->data, externalData, width * height * channels);
			return;
		}
	}
	else
	{
		conprint(1, "Failed to load texture from memory -- invalid data.");
	}
}

Texture::~Texture()
{
	if (cache.size() == 0)
		return;
	auto c = cache.find(file);
	if (c != cache.end())
	{
		Texture* t = c->second;
		t->refCount--;
		if (t->refCount > 0)
			return;
		else
			cache.erase(c);
	}
	glDeleteTextures(1, &ID);
}

void Texture::Use()
{
	Use(0);
}

void Texture::Use(int slot)
{
	if (delayed)
	{
		if (file.empty())
			debprint(3, "Delayed-loading texture on first use...");
		else
			debprint(3, "Delayed-loading texture \"{}\" on first use...", file);
		if (!load(data, &ID, width, height, channels, repeat, filter))
		{
			debprint(2, "glGenTextures indicates we're still threading! WTF?");
			return;
		}
		delete data;
		delayed = false;
	}

	if (currentTexture[slot] != ID)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, ID);
		currentTexture[slot] = ID;
	}
}

void Texture::SetRepeat(int newRepeat)
{
	repeat = newRepeat;
	if (delayed)
		return;
	glBindTexture(GL_TEXTURE_2D, ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::SetFilter(int newFilter)
{
	filter = newFilter;
	if (delayed)
		return;
	glBindTexture(GL_TEXTURE_2D, ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glBindTexture(GL_TEXTURE_2D, 0);
}

//------------------------

static bool loadArray(unsigned char** data, unsigned int *id, int width, int height, int channels, int layers, int repeat, int filter)
{
	glGenTextures(1, id);
	if (glGetError())
		return false;

	int format = (channels == 4) ? GL_RGBA : ((channels == 1) ? GL_RED : GL_RGB);

	glBindTexture(GL_TEXTURE_2D_ARRAY, *id);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, repeat);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, repeat);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, filter);

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, width, height, layers, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	for (auto l = 0; l < layers; l++)
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, l, width, height, 1, format, GL_UNSIGNED_BYTE, data[l]);

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	return true;
}

TextureArray::TextureArray(const std::vector<std::string>& entries, int repeat, int filter) : repeat(repeat), file(entries[0])
{
	ID = 0;
	width = height = channels = 0, layers = 0;
	data = nullptr;

	this->filter = filter == 0 ? BECKETT_DEFAULTFILTER : filter;

	auto c = cacheArray.find(file);
	if (c != cacheArray.end())
	{
		TextureArray* t = c->second;
		ID = t->ID;
		width = t->width;
		height = t->height;
		channels = t->channels;
		data = t->data;
		delayed = t->delayed;
		this->repeat = t->repeat;
		this->filter = t->filter;
		t->refCount++;
		refCount = t->refCount;
		return;
	}

	refCount = 1;

	stbi_set_flip_vertically_on_load(1);

	layers = (int)entries.size();

	data = new unsigned char*[layers] { 0 };
	for (auto l = 0; l < layers; l++)
	{
		size_t vfsSize = 0;
		auto vfsData = VFS::ReadData(entries[l], &vfsSize);
		if (!vfsData || vfsSize == 0)
		{
			conprint(1, "Failed to load texture array item \"{}\" -- no data.", entries[l]);
			return;
		}
		data[l] = stbi_load_from_memory(reinterpret_cast<unsigned char*>(vfsData.get()), (int)vfsSize, &width, &height, &channels, 0);
	}

	if (!loadArray(data, &ID, width, height, channels, layers, repeat, this->filter))
	{
		conprint(3, "glGenTextures indicates we're threading. Delaying \"{}\"...", file);
		delayed = true;
		return;
	}
	for (auto l = 0; l < layers; l++)
		stbi_image_free(data[l]);
	//std::free(data);
	delete[] data;

	cacheArray[file] = this;
}

TextureArray::TextureArray(const std::string& texturePath, int repeat, int filter) : file(texturePath), repeat(repeat)
{
	ID = 0;
	width = height = channels = 0, layers = 0;
	data = nullptr;

	this->filter = filter == 0 ? BECKETT_DEFAULTFILTER : filter;

	auto c = cacheArray.find(texturePath);
	if (c != cacheArray.end())
	{
		TextureArray* t = c->second;
		ID = t->ID;
		width = t->width;
		height = t->height;
		channels = t->channels;
		data = t->data;
		delayed = t->delayed;
		this->repeat = t->repeat;
		this->filter = t->filter;
		t->refCount++;
		refCount = t->refCount;
		return;
	}

	refCount = 1;

	stbi_set_flip_vertically_on_load(1);

	auto entries = VFS::Enumerate(texturePath);

	if (entries.empty())
	{
		std::string tp = texturePath;
		ReplaceAll(tp, "*", "");
		entries = VFS::Enumerate(tp);
	}

	layers = (int)entries.size();
	if (layers == 0)
	{
		//No textures found at all.
		cacheArray[file] = this;
		return;
	}

	data = new unsigned char*[layers] { 0 };
	for (auto l = 0; l < layers; l++)
	{
		size_t vfsSize = 0;
		auto vfsData = VFS::ReadData(entries[l].path, &vfsSize);
		if (!vfsData || vfsSize == 0)
		{
			conprint(1, "Failed to load texture array item \"{}\" -- no data.", entries[l].path);
			return;
		}
		data[l] = stbi_load_from_memory(reinterpret_cast<unsigned char*>(vfsData.get()), (int)vfsSize, &width, &height, &channels, 0);
	}

	if (!loadArray(data, &ID, width, height, channels, layers, repeat, this->filter))
	{
		conprint(3, "glGenTextures indicates we're threading. Delaying \"{}\"...", texturePath);
		delayed = true;
		return;
	}
	for (auto l = 0; l < layers; l++)
		stbi_image_free(data[l]);
	//std::free(data);
	delete[] data;

	cacheArray[file] = this;
}

TextureArray::~TextureArray()
{
	if (cacheArray.size() == 0)
		return;
	auto c = cacheArray.find(file);
	if (c != cacheArray.end())
	{
		//BUGBUG
		if (file == "white.png" || file == "fallback.png" || file == "fallback_nrm.png")
			return;
		TextureArray* t = c->second;
		t->refCount--;
		if (t->refCount > 0)
			return;
		else
			cacheArray.erase(c);
	}
	glDeleteTextures(1, &ID);
}

void TextureArray::Use()
{
	Use(0);
}

void TextureArray::Use(int slot)
{
	if (delayed)
	{
		debprint(3, "Delayed-loading texture array \"{}\" on first use...", file);
		if (!loadArray(data, &ID, width, height, channels, layers, repeat, filter))
		{
			debprint(2, "glGenTextures indicates we're still threading! WTF?");
			return;
		}
		for (auto l = 0; l < layers; l++)
			stbi_image_free(data[l]);
		delete[] data;
		delayed = false;
	}

	if (currentTexture[slot] != ID)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D_ARRAY, ID);
		currentTexture[slot] = ID;
	}
}

void TextureArray::SetRepeat(int newRepeat)
{
	repeat = newRepeat;
	if (delayed)
		return;
	glBindTexture(GL_TEXTURE_2D_ARRAY, ID);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, repeat);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, repeat);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void TextureArray::SetFilter(int newFilter)
{
	filter = newFilter;
	if (delayed)
		return;
	glBindTexture(GL_TEXTURE_2D_ARRAY, ID);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, filter);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}
