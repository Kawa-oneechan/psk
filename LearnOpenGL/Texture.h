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

	Texture() = default;

	Texture(const std::string& texturePath, bool mipmaps = true, int repeat = GL_REPEAT, int filter = GL_LINEAR);
	~Texture();
	Texture(unsigned int existingID, int existingWidth, int existingHeight, int existingChannels);
	void Use();
	void Use(int slot);
};
