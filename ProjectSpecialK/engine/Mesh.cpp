#include "../Game.h"
#ifndef BECKETT_NO3DMODELS
#include <array>
#include <algorithm>
#include <ufbx.h>
#include "Model.h"
#include "Shader.h"
#include "Utilities.h"

Model::Mesh::Mesh(ufbx_mesh* mesh, const Armature& bones, size_t boneCt) : Name(mesh->name.data), Visible(true), Layer(0), Translucent(false), Opaque(false)
{
	Hash = MatHash = GetCRC(Name);

	{
		auto lastUnder = Name.rfind('_');
		if (lastUnder != std::string::npos)
			MatHash = GetCRC(Name.substr(lastUnder + 1));
	}

	Shader = Shaders["model"]; //by default
	std::fill_n(Textures, 4, nullptr);

	std::vector<unsigned int> tri_indices;
	tri_indices.resize(mesh->max_face_triangles * 3);

	bool isSkinned = mesh->skin_deformers.count > 0;
	auto skinDeformer = isSkinned ? mesh->skin_deformers.data[0] : nullptr;
	auto skinVerts = skinDeformer ? skinDeformer->vertices : ufbx_skin_vertex_list();
	auto skinWeights = skinDeformer ? skinDeformer->weights : ufbx_skin_weight_list();

	std::vector<int> vertBones;
	std::vector<float> vertWeights;

	std::array<int, MaxBones> boneMap;
	boneMap.fill(-1);
	if (skinDeformer)
	{
		for (int i = 0; i < skinDeformer->clusters.count; i++)
		{
			std::string clusterName = skinDeformer->clusters[i]->name.data;
			const auto& it = std::find_if(bones.begin(), bones.end(), [clusterName](const auto& e) {
				return e.Name == clusterName;
			});
			if (it != bones.end())
			{
				boneMap[i] = (int)(it - bones.begin());
			}
		}
		for (int i = 0; i < mesh->num_vertices; i++)
		{
			int vBones[MaxWeights];
			float vWeights[MaxWeights];

			int numWeights = 0;
			float totalWeight = 0.0f;

			//A presumption to start with.
			for (int wi = 0; wi < MaxWeights; wi++)
			{
				vBones[wi] = NoBone;
				vWeights[wi] = 0.0f;
			}

			auto skinVert = skinVerts.data[i];
			for (unsigned int wi = 0; wi < skinVert.num_weights; wi++)
			{
				if (numWeights >= MaxWeights) break;
				auto skinWeight = skinWeights.data[skinVert.weight_begin + wi];
				if (skinWeight.cluster_index < MaxBones)
				{
					totalWeight += (float)skinWeight.weight;
					vBones[numWeights] = boneMap[skinWeight.cluster_index];
					vWeights[numWeights] = (float)skinWeight.weight;
					numWeights++;
				}
			}

			for (int wi = 0; wi < MaxWeights; wi++)
			{
				vertBones.push_back(vBones[wi]);
				vertWeights.push_back(vWeights[wi]);
			}
		}
	}

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

			if (isSkinned)
			{
				const int* bv1 = &vertBones[mesh->vertex_indices.data[index] * MaxWeights];
				const float* bv2 = &vertWeights[mesh->vertex_indices.data[index] * MaxWeights];
				for (int wi = 0; wi < MaxWeights; wi++)
				{
					v.Bones[wi] = bv1[wi];
					v.Weights[wi] = bv2[wi];
				}
			}
			else
			{
				for (int wi = 0; wi < MaxWeights; wi++)
				{
					v.Bones[wi] = NoBone;
					v.Weights[wi] = 0.0f;
				}
			}

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

	glVertexAttribIPointer(4, 4, GL_INT, sizeof(Vertex), offsetof(Vertex, Bones));
	glEnableVertexAttribArray(4);

	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, Weights));
	glEnableVertexAttribArray(5);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

#endif
