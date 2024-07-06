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
		int texture;

		Mesh(ufbx_mesh* mesh);
		const void Draw();
	};

private:
	std::string file;
	Texture fallback{ Texture("fallback.png") };

public:
	std::vector<Mesh> Meshes;
	std::array<Texture*, 16> Textures;

	Model() = default;
	Model(const std::string& modelPath);

	void Draw();
};
