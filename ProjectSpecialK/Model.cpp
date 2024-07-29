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
			//auto t = mesh->vertex_tangent[index];

			Vertex v;
			v.Position = glm::vec3(p.x, p.y, p.z);
			v.Normal = glm::vec3(n.x, n.y, n.z);
			v.TexCoords = glm::vec2(u.x, u.y);
			//v.Tangent = glm::vec3(t.x, t.y, t.z);
			vertices.push_back(v);
		}
	}

	for (size_t i = 0; i < vertices.size(); i += 3)
	{
		auto v0 = vertices[i + 0].Position;
		auto v1 = vertices[i + 1].Position;
		auto v2 = vertices[i + 2].Position;
		auto t0 = vertices[i + 0].TexCoords;
		auto t1 = vertices[i + 1].TexCoords;
		auto t2 = vertices[i + 2].TexCoords;

		auto e1 = glm::vec3(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
		auto e2 = glm::vec3(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);

		auto du1 = t1.x - t0.x;
		auto dv1 = t1.y - t0.y;
		auto du2 = t2.x - t0.x;
		auto dv2 = t2.y - t0.y;

		auto f = 1.0f / (du1 * dv2 - du2 * dv1);

		glm::vec3 t;
		t.x = f * (dv2 * e1.x - dv1 * e2.x);
		t.y = f * (dv2 * e1.y - dv1 * e2.y);
		t.z = f * (dv2 * e1.z - dv1 * e2.z);
		t = glm::normalize(t);

		vertices[i + 0].Tangent = t;
		vertices[i + 1].Tangent = t;
		vertices[i + 2].Tangent = t;

		/*
		glm::vec3 b;
		b.x = f * (-du2 * e1.x + du1 * e2.x);
		b.y = f * (-du2 * e1.y + du1 * e2.y);
		b.z = f * (-du2 * e1.z + du1 * e2.z);
		t = glm::normalize(b);

		vertices[i + 0].BiTangent = b;
		vertices[i + 1].BiTangent = b;
		vertices[i + 2].BiTangent = b;
		*/
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

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, Tangent));
	glEnableVertexAttribArray(3);

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
	Textures.fill(nullptr);

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
			auto texNum = m.texture * 4;
			for (auto i = 0; i < 4; i++)
			{
				if (Textures[texNum + i] == nullptr)
				{
					if (i == 0)
						fallback.Use(0);
					else
						whiteRect->Use(i);
				}
				else
				{
					Textures[texNum + i]->Use(i);
				}
			}
		}
		m.Draw();
	}

	glBindVertexArray(0);
}
