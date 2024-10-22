#include "SpecialK.h"
#include "Model.h"
#include "support/ufbx.h"

static std::map<std::string, std::tuple<Model*, int>> cache;
extern Shader* modelShader;

static std::map<std::string, unsigned int> matMap;

Model::Mesh::Mesh(ufbx_mesh* mesh) : Texture(-1), Visible(true)
{
	Hash = GetCRC(mesh->name.data);

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
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
}

Model::Model(const std::string& modelPath) : file(modelPath)
{
	Textures.fill(nullptr);
	TexArrayLayers.fill(0);

	if (matMap.empty())
	{
		auto mapJson = VFS::ReadJSON("matmap.json");
		for (auto& item : mapJson->AsObject())
		{
			matMap.emplace(item.first, item.second->AsInteger());
		}
		delete mapJson;
	}

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
	auto vfsData = VFS::ReadData(modelPath, &vfsSize);

	ufbx_load_opts options = {};
	ufbx_error errors;
	ufbx_scene *scene = ufbx_load_memory(vfsData.get(), vfsSize, &options, &errors);
	if (!scene)
		FatalError(fmt::format("Could not load scene {}: {}", modelPath, errors.description.data));

	conprint(5, "Loading {}\n-------------------------------", modelPath);

	conprint(5, "Materials:");
	for (auto m : scene->materials)
	{
		conprint(0, "* {}", m->name.data);
	}

	conprint(5, "Meshes:");
	unsigned int matCt = 0;
	for (size_t i = 0; i < scene->nodes.count; i++)
	{
		ufbx_node *node = scene->nodes.data[i];
		if (node->mesh)
		{
			auto m = Mesh(node->mesh);
			m.Texture = -1;
			if (node->mesh->materials.count > 0)
			{
				auto& m1 = node->mesh->materials[0]->name.data;
				auto foundIt = false;
				for (auto it : matMap)
				{
					if (it.first == m1)
					{
						m.Texture = it.second;
						conprint(0, "* {} ({} > {})", node->name.data, it.first, it.second);
						foundIt = true;
						break;
					}
				}
				if (!foundIt)
				{
					m.Texture = matCt;
					conprint(0, "* {} (#{})", node->name.data, matCt);
					matCt++;
				}
			}
			Meshes.emplace_back(m);
		}
	}

	ufbx_free_scene(scene);

	cache[file] = std::make_tuple(this, 1);
}

void Model::Draw(Shader* shader, const glm::vec3& pos, float yaw)
{
	shader->Use();

	auto projection = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 100.0f);
	auto view = MainCamera.ViewMat();

	shader->Set("view", view);
	shader->Set("projection", projection);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, pos);
	model = glm::rotate(model, glm::radians(yaw), glm::vec3(0, 1, 0));
	shader->Set("model", model);

	int j = 0;
	for (auto& m : Meshes)
	{
		if (!m.Visible)
			continue;

		if (m.Texture != -1 && m.Texture < Textures.size())
		{
			auto texNum = m.Texture * 4;
			for (auto i = 0; i < 4; i++)
			{
				if (Textures[texNum + i] == nullptr)
				{
					if (i == 0)
						fallback.Use(0);
					else
						white.Use(i);
				}
				else
				{
					Textures[texNum + i]->Use(i);
				}
			}
		}
		shader->Set("layer", TexArrayLayers[j]);
		m.Draw();
		j++;
	}

	glBindVertexArray(0);
}

void Model::AllVisible()
{
	for (auto& m : Meshes)
		m.Visible = true;
}

Model::Mesh& Model::GetMesh(const std::string& name)
{
	auto hash = GetCRC(name);
	for (auto& m : Meshes)
	{
		if (m.Hash == hash)
			return m;
	}
	throw std::runtime_error(fmt::format("Model::GetMesh(): could not find a mesh with name \"{}\" in \"{}\".", name, file));
}
