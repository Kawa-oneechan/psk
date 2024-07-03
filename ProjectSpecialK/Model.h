#pragma once

#include "support/glad/glad.h"
#include "support/stb_image.h"
#include "support/format.h"
#include "VFS.h"

class Model
{
private:
	std::string file;

public:
	unsigned int VBO, VAO, EBO;

	Model() = default;
	Model(const std::string& modelPath);

	void Draw();
};
