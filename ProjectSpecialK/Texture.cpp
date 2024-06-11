#include "SpecialK.h"

//TODO: refactor the actual loady bits.

Texture::Texture(const std::string& texturePath, bool mipmaps, int repeat, int filter)
{
	ID = 0xDEADBEEF;
	width = height = channels = 0;

	stbi_set_flip_vertically_on_load(1);

	size_t vfsSize = 0;
	char* vfsData = ReadVFS(texturePath, &vfsSize);
	if (vfsData == nullptr || vfsSize == 0)
	{
		conprint(1, "Failed to load texture \"{}\" -- no data.", texturePath);
		return;
	}
	unsigned char *data = stbi_load_from_memory((unsigned char*)vfsData, (int)vfsSize, &width, &height, &channels, 0);
	free(vfsData);

	int format = GL_RGB;
	if (channels == 4) format = GL_RGBA;

	if (data)
	{
		glGenTextures(1, &ID);
		if (ID == 0xDEADBEEF)
		{
			//We are delayed by multithreading!
			conprint(3, "glGenTextures indicates we're threading. Delaying \"{}\"...", texturePath);
			delayed = true;
			delayedData = data;
			delayedFilter = filter;
			delayedRepeat = repeat;
			return;
		}
		glBindTexture(GL_TEXTURE_2D, ID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		if (mipmaps)
			glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else
	{
		conprint(1, "Failed to load texture \"{}\" -- invalid data.", texturePath);
	}
	stbi_image_free(data);
}

Texture::Texture(const unsigned char* data, int existingWidth, int existingHeight, int existingChannels, bool mipmaps, int repeat, int filter)
{
	ID = 0xDEADBEEF;
	width = existingWidth;
	height = existingHeight;
	channels = existingChannels;

	int format = GL_RGB;
	if (channels == 4) format = GL_RGBA;

	if (data)
	{
		glGenTextures(1, &ID);
		auto err = glGetError();
		if (ID == 0xDEADBEEF)
		{
			//We are delayed by multithreading!
			conprint(3, "glGenTextures indicates we're threading. Delaying load from memory...");
			delayed = true;
			delayedData = (unsigned char*)data;
			delayedFilter = filter;
			delayedRepeat = repeat;
			return;
		}
		glBindTexture(GL_TEXTURE_2D, ID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

		int format = GL_RGB;
		if (channels == 4) format = GL_RGBA;

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		err = glGetError();
		if (mipmaps)
			glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		err = glGetError();
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

Texture::Texture(unsigned int existingID, int existingWidth, int existingHeight, int existingChannels)
{
	ID = existingID;
	width = existingWidth;
	height = existingHeight;
	channels = existingChannels;
}

void Texture::Use()
{
	Use(0);
}

void Texture::Use(int slot)
{
	if (delayed)
	{
		conprint(3, "Delayed-loading texture on first use...")
		glGenTextures(1, &ID);
		auto err = glGetError();
		if (ID == 0xDEADBEEF)
		{
			//We are delayed by multithreading!
			conprint(2, "glGenTextures indicates we're still threading! WTF?");
			return;
		}
		glBindTexture(GL_TEXTURE_2D, ID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, delayedRepeat);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, delayedRepeat);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, delayedFilter);

		int format = GL_RGB;
		if (channels == 4) format = GL_RGBA;

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, delayedData);
		err = glGetError();
		//if (mipmaps)
			glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		err = glGetError();
		delete delayedData;
		delayed = false;
	}

	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, ID);
}
