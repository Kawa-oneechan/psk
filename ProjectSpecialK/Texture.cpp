#include "Texture.h"

Texture::Texture(const std::string& texturePath, bool mipmaps, int repeat, int filter)
{
	ID = 0;
	width = height = channels = 0;

	stbi_set_flip_vertically_on_load(1);

	size_t vfsSize = 0;
	char* vfsData = ReadVFS(texturePath, &vfsSize);
	if (vfsData == nullptr || vfsSize == 0)
	{
		fmt::print("Failed to load texture \"{}\" -- no data\n", texturePath);
		return;
	}
	unsigned char *data = stbi_load_from_memory((unsigned char*)vfsData, (int)vfsSize, &width, &height, &channels, 0);
	free(vfsData);

	int format = GL_RGB;
	if (channels == 4) format = GL_RGBA;

	if (data)
	{
		glGenTextures(1, &ID);
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
		fmt::print("Failed to load texture \"{}\" -- invalid data\n", texturePath);
	}
	stbi_image_free(data);
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
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::Use(int slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, ID);
}
