#pragma once

#include "support/glad/glad.h"
#include "support/stb_image.h"
#include "support/format.h"
#include "VFS.h"

class Texture
{
public:
	unsigned int ID;
	int width, height, channels;

	//Creates a Texture from a given VFS path.
	Texture(const char* texturePath, bool mipmaps = true)
	{
		glGenTextures(1, &ID);
		glBindTexture(GL_TEXTURE_2D, ID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_set_flip_vertically_on_load(1);

		size_t vfsSize = 0;
		char* vfsData = ReadVFS(texturePath, &vfsSize);
		unsigned char *data = stbi_load_from_memory((unsigned char*)vfsData, (int)vfsSize, &width, &height, &channels, 0);
		free(vfsData);

		int dataType = channels == 4 ? GL_RGBA : GL_RGB;

		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, dataType, width, height, 0, dataType, GL_UNSIGNED_BYTE, data);
			if (mipmaps)
				glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			fmt::print("Failed to load texture \"{}\".\n", texturePath);
		}
		stbi_image_free(data);
	}
	
	//Creates a Texture from memory.
	Texture(unsigned char* data, int w, int h, int nc, bool mipmaps = true)
	{
		width = w;
		height = h;
		channels = nc;

		glGenTextures(1, &ID);
		glBindTexture(GL_TEXTURE_2D, ID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		int dataType = channels == 4 ? GL_RGBA : GL_RGB;

		glTexImage2D(GL_TEXTURE_2D, 0, dataType, width, height, 0, dataType, GL_UNSIGNED_BYTE, data);
		if (mipmaps)
			glGenerateMipmap(GL_TEXTURE_2D);
	}

	void use()
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ID);
	}

	void use(int slot)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, ID);
	}

};