#include "SpecialK.h"

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

	stbi_set_flip_vertically_on_load(1);

	size_t vfsSize = 0;
	char* vfsData = ReadVFS(texturePath, &vfsSize);
	if (vfsData == nullptr || vfsSize == 0)
	{
		conprint(1, "Failed to load texture \"{}\" -- no data.", texturePath);
		return;
	}
	data = stbi_load_from_memory((unsigned char*)vfsData, (int)vfsSize, &width, &height, &channels, 0);
	free(vfsData);

	if (data)
	{
		if (!load(data, &ID, width, height, channels, repeat, filter))
		{
			//We are delayed by multithreading!
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
}

Texture::Texture(const unsigned char* externalData, int width, int height, int channels, int repeat, int filter) : data(nullptr), width(width), height(height), channels(channels), repeat(repeat), filter(filter)
{
	ID = 0;
	this->file.clear();

	int format = GL_RGB;
	if (channels == 4) format = GL_RGBA;

	if (externalData)
	{
		if (!load(externalData, &ID, width, height, channels, repeat, filter))
		{
			//We are delayed by multithreading!
			conprint(3, "glGenTextures indicates we're threading. Delaying load from memory...");
			delayed = true;
			this->data = (unsigned char*)externalData;
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
	GLuint temp[] = { ID };
	glDeleteTextures(1, temp);
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
			//We are delayed by multithreading!
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