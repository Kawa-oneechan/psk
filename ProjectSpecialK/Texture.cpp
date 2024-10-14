#include "SpecialK.h"
#include "support/stb_image.h"

static std::map<std::string, std::tuple<Texture*, int>> cache;
static std::map<std::string, std::tuple<TextureArray*, int>> cacheArray;

static bool load(const unsigned char* data, unsigned int *id, int width, int height, int channels, int repeat, int filter)
{
	glGenTextures(1, id);
	if (glGetError())
		return false;

	int format = (channels == 4) ? GL_RGBA : GL_RGB;

	glBindTexture(GL_TEXTURE_2D, *id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

Texture::Texture(const std::string& texturePath, int repeat, int filter) : file(texturePath), repeat(repeat), filter(filter)
{
	ID = 0;
	width = height = channels = 0;
	data = nullptr;

	auto c = cache.find(texturePath);
	if (c != cache.end())
	{
		Texture* t;
		int r;
		std::tie(t, r) = c->second;
		ID = t->ID;
		width = t->width;
		height = t->height;
		channels = t->channels;
		data = t->data;
		delayed = t->delayed;
		this->repeat = t->repeat;
		this->filter = t->filter;
		atlas = t->atlas;
		r++;
		cache[file] = std::make_tuple(t, r);
		return;
	}

	stbi_set_flip_vertically_on_load(1);

	size_t vfsSize = 0;
	auto vfsData = VFS::ReadData(texturePath, &vfsSize);
	if (!vfsData || vfsSize == 0)
	{
		conprint(1, "Failed to load texture \"{}\" -- no data.", texturePath);
		return;
	}
	data = stbi_load_from_memory((unsigned char*)vfsData.get(), (int)vfsSize, &width, &height, &channels, 0);

	auto atlasPath = texturePath.substr(0, texturePath.find_last_of('.')) + ".json";
	GetAtlas(atlas, atlasPath);
	if (atlas.empty())
		atlas.push_back(glm::vec4(0, 0, width, height));

	if (data)
	{
		if (!load(data, &ID, width, height, channels, repeat, filter))
		{
			conprint(3, "glGenTextures indicates we're threading. Delaying \"{}\"...", texturePath);
			delayed = true;
			return;
		}
	}
	else
	{
		conprint(1, "Failed to load texture \"{}\" -- invalid data.", texturePath);
	}
	stbi_image_free(data);

	cache[file] = std::make_tuple(this, 1);
}

Texture::Texture(const unsigned char* externalData, int width, int height, int channels, int repeat, int filter) : data(nullptr), width(width), height(height), channels(channels), repeat(repeat), filter(filter)
{
	ID = 0;
	this->file.clear();

	atlas.push_back(glm::vec4(0, 0, width, height));

	int format = GL_RGB;
	if (channels == 4) format = GL_RGBA;

	if (externalData)
	{
		if (!load(externalData, &ID, width, height, channels, repeat, filter))
		{
			conprint(3, "glGenTextures indicates we're threading. Delaying load from memory...");
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
		Texture* t;
		int r;
		std::tie(t, r) = c->second;
		r--;
		if (r > 0)
		{
			cache[file] = std::make_tuple((Texture*)this, r);
			return;
		}
		else
		{
			cache.erase(c);
		}
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
			conprint(3, "Delayed-loading texture on first use...");
		else
			conprint(3, "Delayed-loading texture \"{}\" on first use...", file);
		if (!load(data, &ID, width, height, channels, repeat, filter))
		{
			conprint(2, "glGenTextures indicates we're still threading! WTF?");
			return;
		}
		delete data;
		delayed = false;
	}

	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, ID);
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

//------------------------

static bool loadArray(unsigned char** data, unsigned int *id, int width, int height, int channels, int layers, int repeat, int filter)
{
	glGenTextures(1, id);
	if (glGetError())
		return false;

	int format = (channels == 4) ? GL_RGBA : GL_RGB;

	glBindTexture(GL_TEXTURE_2D_ARRAY, *id);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, repeat);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, repeat);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, filter);

	//glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, format, width, height, layers, 0, format, GL_UNSIGNED_BYTE, nullptr);

	for (auto l = 0; l < layers; l++)
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, l, width, height, 1, format, GL_UNSIGNED_BYTE, data[l]);

	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	return true;
}

TextureArray::TextureArray(const std::string& texturePath, int repeat, int filter) : file(texturePath), repeat(repeat), filter(filter)
{
	ID = 0;
	width = height = channels = 0, layers = 0;
	data = nullptr;

	auto c = cacheArray.find(texturePath);
	if (c != cacheArray.end())
	{
		TextureArray* t;
		int r;
		std::tie(t, r) = c->second;
		ID = t->ID;
		width = t->width;
		height = t->height;
		channels = t->channels;
		data = t->data;
		delayed = t->delayed;
		this->repeat = t->repeat;
		this->filter = t->filter;
		r++;
		cacheArray[file] = std::make_tuple(t, r);
		return;
	}

	stbi_set_flip_vertically_on_load(1);

	auto entries = VFS::Enumerate(texturePath);
	layers = (int)entries.size();
	data = (unsigned char**)std::malloc(layers * sizeof(unsigned char**));
	if (data)
	{
		std::memset(data, 0, layers);
		for (auto l = 0; l < layers; l++)
		{
			size_t vfsSize = 0;
			auto vfsData = VFS::ReadData(entries[l].path, &vfsSize);
			if (!vfsData || vfsSize == 0)
			{
				conprint(1, "Failed to load texture array item \"{}\" -- no data.", entries[l].path);
				return;
			}
			data[l] = stbi_load_from_memory((unsigned char*)vfsData.get(), (int)vfsSize, &width, &height, &channels, 0);
		}

		if (!loadArray(data, &ID, width, height, channels, layers, repeat, filter))
		{
			conprint(3, "glGenTextures indicates we're threading. Delaying \"{}\"...", texturePath);
			delayed = true;
			return;
		}
	}
	else
	{
		conprint(1, "Failed to load texture \"{}\" -- couldn't allocate data.", texturePath);
	}
	for (auto l = 0; l < layers; l++)
		stbi_image_free(data[l]);
	std::free(data);

	cacheArray[file] = std::make_tuple(this, 1);
}

TextureArray::~TextureArray()
{
	if (cacheArray.size() == 0)
		return;
	auto c = cacheArray.find(file);
	if (c != cacheArray.end())
	{
		TextureArray* t;
		int r;
		std::tie(t, r) = c->second;
		r--;
		if (r > 0)
		{
			cacheArray[file] = std::make_tuple((TextureArray*)this, r);
			return;
		}
		else
		{
			cacheArray.erase(c);
		}
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
		conprint(3, "Delayed-loading texture array \"{}\" on first use...", file);
		if (!loadArray(data, &ID, width, height, channels, layers, repeat, filter))
		{
			conprint(2, "glGenTextures indicates we're still threading! WTF?");
			return;
		}
		for (auto l = 0; l < layers; l++)
			stbi_image_free(data[l]);
		std::free(data);
		delayed = false;
	}

	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D_ARRAY, ID);
}
