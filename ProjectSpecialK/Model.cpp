#include "SpecialK.h"
#include "Model.h"
#include "Camera.h"
#include "support/ufbx.h"

static std::map<std::string, std::tuple<Model*, int>> cache;
extern Shader* modelShader;
extern Camera camera;

Model::Mesh::Mesh(ufbx_mesh* mesh) : texture(-1)
{
	std::vector<unsigned int> tri_indices;
	tri_indices.resize(mesh->max_face_triangles * 3);

	for (auto face : mesh->faces)
	{
		uint32_t num_tris = ufbx_triangulate_face(tri_indices.data(), tri_indices.size(), mesh, face);

		for (size_t i = 0; i < num_tris * 3; i++)
		{
			auto index = tri_indices[i];
			
			auto p = mesh->vertex_position[index];
			auto n = mesh->vertex_normal[index];
			auto u = mesh->vertex_uv[index];

			Vertex v;
			v.Position = glm::vec3(p.x, p.y, p.z) * 0.05f;
			v.Normal = glm::vec3(n.x, n.y, n.z);
			v.TexCoords = glm::vec2(u.x, u.y);
			vertices.push_back(v);
		}
	}

	ufbx_vertex_stream streams[1] = {
		{ vertices.data(), vertices.size(), sizeof(Vertex) },
	};
	
	indices.resize(mesh->num_triangles * 3);
	auto num_vertices = ufbx_generate_indices(streams, 1, indices.data(), indices.size(), nullptr, nullptr);

	vertices.resize(num_vertices);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, Normal));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, TexCoords));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

const void Model::Mesh::Draw()
{
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 100.0f);

	//glm::mat4 view = glm::mat4(1.0f);
	// note that we're translating the scene in the reverse direction of where we want to move
	//view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
	auto view = camera.GetViewMatrix();

	//ourShader.setMat4("model", model);
	modelShader->SetMat4("view", view);
	modelShader->SetMat4("projection", projection);

	//texture.Use();

	glm::mat4 model = glm::mat4(1.0f);
	//model = glm::translate(model, cubePositions[i]);
	//float angle = 20.0f;
	//angle = (float)glfwGetTime() * 25.0f;
	//model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
	modelShader->SetMat4("model", model);

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);

}

Model::Model(const std::string& modelPath) : file(modelPath)
{
	auto c = cache.find(modelPath);
	if (c != cache.end())
	{
		Model* m;
		int r;
		std::tie(m, r) = c->second;
		Meshes = m->Meshes;
		r++;
		cache[file] = std::make_tuple(m, r);
		return;
	}
	
	size_t vfsSize = 0;
	auto vfsData = ReadVFS(modelPath, &vfsSize);
		
	ufbx_load_opts options = {};
	ufbx_error errors;
	ufbx_scene *scene = ufbx_load_memory(vfsData.get(), vfsSize, &options, &errors);
	if (!scene)
		FatalError(fmt::format("Could not load scene {}: {}", modelPath, errors.description.data));
	
	std::vector<std::string> textureNames;
	for (auto m : scene->materials)
	{
		conprint(5, "{}", m->name.data);
		textureNames.emplace_back(m->name.data);
	}

	for (size_t i = 0; i < scene->nodes.count; i++)
	{
		ufbx_node *node = scene->nodes.data[i];
		if (node->mesh)
		{
			auto m = Mesh(node->mesh);
			m.texture = -1;
			if (node->mesh->materials.count > 0)
			{
				auto m1 = node->mesh->materials[0]->name.data;
				auto it = std::find(textureNames.cbegin(), textureNames.cend(), m1);
				m.texture = (unsigned int)std::distance(textureNames.cbegin(), it);
			}
			Meshes.emplace_back(m);
		}
	}

	ufbx_free_scene(scene);

	cache[file] = std::make_tuple(this, 1);
}

void Model::Draw()
{
	modelShader->Use();
	for (auto& m : Meshes)
	{
		if (m.texture != -1 && m.texture < Textures.size())
		{
			auto texNum = m.texture * 3;
			for (auto i = 0; i < 3; i++)
			{
				if (Textures[texNum + i] == nullptr)
				{
					if (i == 0)
						fallback.Use(0);
					else
						whiteRect->Use(i);
				}
				else
					Textures[texNum + i]->Use(i);
			}
		}
		m.Draw();
	}

	glBindVertexArray(0);
}
