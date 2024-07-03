#pragma once

#include "support/glad/glad.h"
#include "support/stb_image.h"
#include "support/format.h"
#include "support/ufbx.h"
#include "VFS.h"

class Model
{
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
	};

	class Mesh
	{
	public:
		unsigned int VBO, VAO, EBO;
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;

		Mesh(ufbx_mesh* mesh);
		const void Draw();
	};

private:
	std::string file;

public:
	std::vector<Mesh> Meshes;

	Model() = default;
	Model(const std::string& modelPath);

	void Draw();
};
