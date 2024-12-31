#include "SpecialK.h"
#include "Model.h"
#include "support/ufbx.h"

unsigned int currentVAO = 0;

namespace MeshBucket
{
	static constexpr int meshBucketSize = 32;

	struct MeshInABucket
	{
		unsigned int VAO;
		Shader* Shader;
		TextureArray* Textures[4];
		int Layer;
		glm::vec3 Position;
		glm::quat Rotation;
		glm::mat4 Bones[MaxBones];
		size_t BoneCount;
		size_t Indices;
	};

	static int meshesInBucket;
	static std::array<MeshInABucket, meshBucketSize> meshBucket;

	void Flush()
	{
		std::sort(meshBucket.begin(), meshBucket.begin() + meshesInBucket, [](const MeshInABucket& a, const MeshInABucket& b)
		{
			if (a.Shader < b.Shader)
				return true;
			else if (a.Shader > b.Shader)
				return false;
			else if (a.Textures[0] < b.Textures[0])
				return true;
			else if (a.Textures[0] > b.Textures[0])
				return false;
			else if (a.Layer < b.Layer)
				return true;
			else
				return false;
		});

		unsigned int currentShader = (unsigned int)-1;
		glm::vec3 currentPos;
		glm::quat currentRot;
		unsigned int currentTextures[4]{ currentShader, currentShader, currentShader, currentShader };
		int currentLayer = -1;

		for (auto i = 0; i < meshesInBucket; i++)
		{
			auto& m = meshBucket[i];
			if (m.Shader->ID != currentShader)
			{
				currentShader = m.Shader->ID;
				currentLayer = -1;
				m.Shader->Use();
			}

			if (m.Position != currentPos || m.Rotation != currentRot)
			{
				currentPos = m.Position;
				currentRot = m.Rotation;

				auto t = glm::translate(glm::mat4(1), m.Position);
				auto r = (glm::mat4)m.Rotation;
				//auto s = glm::scale(glm::mat4(1), scale);
				auto model = t * r; //* s;

				m.Shader->Set("model", model);
			}

			m.Shader->Set("finalBonesMatrices", m.Bones[0], m.BoneCount);

			for (auto j = 0; j < 4; j++)
			{
				if (m.Textures[j]->ID != currentTextures[j])
				{
					currentTextures[j] = m.Textures[j]->ID;
					m.Textures[j]->Use(j);
				}
			}

			if (m.Layer != currentLayer)
			{
				currentLayer = m.Layer;
				m.Shader->Set("layer", m.Layer);
			}

			if (m.VAO != currentVAO)
			{
				currentVAO = m.VAO;
				glBindVertexArray(m.VAO);
			}
			glDrawElements(GL_TRIANGLES, (GLsizei)m.Indices, GL_UNSIGNED_INT, 0);
		}

		glBindVertexArray(0);
		currentVAO = 0;

		meshesInBucket = 0;
		std::memset(&meshBucket, 0, sizeof(MeshInABucket) * meshBucketSize);
	}

	void Draw(unsigned int vao, TextureArray* textures[], int layer, Shader* shader, const glm::vec3& position, const glm::quat& rotation, const glm::mat4 bones[], size_t indices, size_t boneCt)
	{
		meshBucket[meshesInBucket].VAO = vao;
		meshBucket[meshesInBucket].Shader = shader;
		meshBucket[meshesInBucket].Position = position;
		meshBucket[meshesInBucket].Rotation = rotation;
		meshBucket[meshesInBucket].Indices = indices;
		meshBucket[meshesInBucket].BoneCount = boneCt;
		meshBucket[meshesInBucket].Layer = layer;
		for (auto i = 0; i < 4; i++)
			meshBucket[meshesInBucket].Textures[i] = textures[i];
		for (auto i = 0; i < MaxBones; i++)
			meshBucket[meshesInBucket].Bones[i] = bones[i];

		meshesInBucket++;
		if (meshesInBucket == meshBucketSize)
			Flush();
	};
}

static std::map<std::string, std::tuple<Model*, int>> cache;
extern Shader* modelShader;
extern Shader* grassShader;

//static std::map<std::string, unsigned int> matMap;

Model::Mesh::Mesh(ufbx_mesh* mesh, std::vector<Bone>& bones) : Visible(true)
{
	Hash = GetCRC(mesh->name.data);
	Name = mesh->name.data;
	Shader = modelShader; //by default

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
			for (int j = 0; j < bones.size(); j++)
			{
				if (bones[j].Name == clusterName)
				{
					boneMap[i] = j;
					break;
				}
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
				auto bv1 = &vertBones[mesh->vertex_indices.data[index] * MaxWeights];
				auto bv2 = &vertWeights[mesh->vertex_indices.data[index] * MaxWeights];
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

#pragma warning(push)
#pragma warning(disable: 4838 4244)
static glm::mat4 ufbxToGlmMat4(const ufbx_matrix& mat)
{
	float ret[] = {
		mat.m00, mat.m01, mat.m02, mat.m03,
		mat.m10, mat.m11, mat.m12, mat.m13,
		mat.m20, mat.m21, mat.m22, mat.m23,
		0.0f,    0.0f,    0.0f,    1.0f
	};
	return glm::make_mat4(ret);
}
#pragma warning(pop)

Model::Model(const std::string& modelPath) : file(modelPath)
{
	//Textures.fill(nullptr);
	TexArrayLayers.fill(0);

	/*
	if (matMap.empty())
	{
		auto mapJson = VFS::ReadJSON("matmap.json");
		for (auto& item : mapJson->AsObject())
		{
			matMap.emplace(item.first, item.second->AsInteger());
		}
		delete mapJson;
	}
	*/

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
	
	debprint(5, "Loading {}\n-------------------------------", modelPath);

	auto basePath = modelPath.substr(0, modelPath.rfind('/') + 1);
	auto matMapFile = modelPath.substr(0, modelPath.rfind('.')) + ".mat.json";
	auto matMap = VFS::ReadJSON(matMapFile);
	if (!matMap)
	{
		matMapFile = matMapFile.substr(0, modelPath.rfind('/')) + "/default.mat.json";
		matMap = VFS::ReadJSON(matMapFile);
		while (!matMap)
		{
			if (matMapFile.find('/') != std::string::npos)
			{
				//FatalError("No material map? How?");
				matMap = JSON::Parse("{}");
				break;
			}
			matMapFile = matMapFile.substr(0, matMapFile.rfind('/'));
			matMapFile = matMapFile.substr(0, matMapFile.rfind('/')) + "/default.mat.json";
		}
	}

	debprint(5, "Materials:");
	for (auto m : scene->materials)
	{
		debprint(0, "* {}", m->name.data);
	}

	if (scene->skin_clusters.count > 0)
	{
		debprint(5, "Bones:");
		unsigned int boneCt = 0;
		std::array<int, MaxBones> clusterMap;
		for (auto i = 0; i < scene->skin_clusters.count; i++)
		{
			auto cluster = scene->skin_clusters.data[i];
			auto bone = cluster->bone_node;
			auto boneName = bone->name.data;
			auto exists = false;
			for (auto& b : Bones)
			{
				if (b.Name == boneName)
				{
					exists = true;
					break;
				}
			}
			if (exists)
				continue;
			debprint(0, "* {}. {}", boneCt, boneName);
			auto b = Bone();
			b.Name = boneName;
			b.Offset = ufbxToGlmMat4(bone->geometry_to_node);
			Bones.push_back(b);
			clusterMap[boneCt] = i;
			boneCt++;
		}

		auto boundBoneCt = boneCt;

		for (auto i = 0; i < scene->bones.count; i++)
		{
			auto bone = scene->bones.data[i];
			auto boneName = bone->name.data;
			auto exists = false;
			for (auto& b : Bones)
			{
				if (b.Name == boneName)
				{
					exists = true;
					break;
				}
			}
			if (exists)
				continue;
			debprint(0, "* {}. {} (extra)", boneCt, boneName);
			auto b = Bone();
			b.Name = boneName;
			Bones.push_back(b);
			boneCt++;
		}

		//lap two: parent/child relations
		for (auto i = 0u; i < boundBoneCt; i++)
		{
			auto cluster = scene->skin_clusters.data[clusterMap[i]];
			auto bone = cluster->bone_node;
			auto boneName = bone->name.data;
			if (bone->parent != nullptr)
			{
				auto parentBone = bone->parent->name.data;
				for (auto& pb : Bones)
				{
					if (pb.Name == parentBone)
					{
						pb.Children.push_back(i);
						break;
					}
				}
			}

			Bone* thisBone = nullptr;
			auto thisBoneIdx = 0;
			for (auto& b : Bones)
			{
				if (b.Name == boneName)
				{
					thisBone = &b;
					break;
				}
				thisBoneIdx++;
			}
			if (!thisBone)
				continue;
			for (auto j = 0; j < bone->children.count; j++)
			{
				auto childBone = bone->children.data[j];
				auto childIndex = 0;
				for (auto& b : Bones)
				{
					if (b.Name == childBone->name.data)
						break;
					childIndex++;
				}
				auto bur = false;
				for (auto kek : thisBone->Children)
				{
					if (kek == childIndex)
					{
						bur = true;
						break;
					}
				}
				if (!bur)
					thisBone->Children.push_back(childIndex);
			}
		}

		for (int i = 0; i < Bones.size() /*MaxBones*/; i++)
			finalBoneMatrices[i] = glm::identity<glm::mat4>(); //Bones[i].Offset; //glm::mat4(1.0f);
	}

	debprint(5, "Meshes:");
	unsigned int matCt = 0;
	for (size_t i = 0; i < scene->nodes.count; i++)
	{
		ufbx_node *node = scene->nodes.data[i];
		if (node->mesh)
		{
			auto m = Mesh(node->mesh, Bones);
			m.Shader = modelShader;
			m.Textures[0] = &fallback;
			m.Textures[1] = &white;
			m.Textures[2] = &white;
			m.Textures[3] = &white;

			if (node->mesh->materials.count > 0)
			{
				auto& m1 = node->mesh->materials[0]->name.data;
				bool foundIt = false;
				for (auto it : matMap->AsObject())
				{
					if (it.first == m1)
					{
						auto mat = it.second->AsObject();
						if (mat["shader"])
						{
							auto s = mat["shader"]->AsString();
							if (s == "grass")
								m.Shader = grassShader;
						}
						if (mat["albedo"])
							m.Textures[0] = new TextureArray(basePath + mat["albedo"]->AsString());
						if (mat["normal"])
							m.Textures[1] = new TextureArray(basePath + mat["normal"]->AsString());
						if (mat["mix"])
							m.Textures[2] = new TextureArray(basePath + mat["mix"]->AsString());
						if (mat["opacity"])
							m.Textures[3] = new TextureArray(basePath + mat["opacity"]->AsString());

						debprint(0, "* #{}: {} > {}",  matCt, node->name.data, m1);
						foundIt = true;
						break;
					}
				}
				if (!foundIt)
					debprint(0, "* #{}: {} > {} (Unmapped)", matCt, node->name.data, m1);
				matCt++;
			}
			/*
			m.Texture = -1;
			if (node->mesh->materials.count > 0)
			{
				auto& m1 = node->mesh->materials[0]->name.data;
				auto foundIt = false;
				for (auto it : matMap)
				{
					if (it.first == m1)
					{
						//m.Texture = it.second;
						debprint(0, "* {} ({} > {})", node->name.data, it.first, it.second);
						foundIt = true;
						break;
					}
				}
				if (!foundIt)
				{
					//m.Texture = matCt;
					debprint(0, "* {} (#{})", node->name.data, matCt);
					matCt++;
				}
			}
			*/
			Meshes.emplace_back(m);
		}
	}

	ufbx_free_scene(scene);

	cache[file] = std::make_tuple(this, 1);
}

void Model::Draw(const glm::vec3& pos, float yaw, int mesh)
{
	/*
	if (Bones.size() > 0)
	{
		//TEST TEST TEST TEST
		auto head = FindBone("Head");
		if (head != NoBone)
			MoveBone(head, glm::vec3(0, glm::radians(11.0f), 0));
		CalculateBoneTransform(0);
		//TEST TEST TEST TEST
	}
	*/

	int j = 0;
	TextureArray* texs[4];
	for (int i = 0; i < 4; i++)
		texs[i] = nullptr;

	for (auto& m : Meshes)
	{
		if (!m.Visible)
			continue;
		if (mesh != -1 && mesh != j)
		{
			j++;
			continue;
		}

		/*
		if (m.Texture != -1 && m.Texture < Textures.size())
		{
			auto texNum = m.Texture * 4;
			for (auto i = 0; i < 4; i++)
			{
				if (Textures[texNum + i] == nullptr)
				{
					if (i == 0)
						texs[0] = &fallback;
					else
						texs[i] = &white;
				}
				else
				{
					texs[i] = Textures[texNum + i];
				}
			}
		}
		*/
		glm::vec3 r(0, glm::radians(yaw), 0);
		//MeshBucket::Draw(m.VAO, texs, TexArrayLayers[j], m.Shader, pos, glm::quat(r), finalBoneMatrices, m.Indices(), Bones.size());
		MeshBucket::Draw(m.VAO, m.Textures, TexArrayLayers[j], m.Shader, pos, glm::quat(r), finalBoneMatrices, m.Indices(), Bones.size());
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

Model::Mesh& Model::GetMesh(int index)
{
	if (index >= 0 && index < Meshes.size())
		return Meshes[index];
	throw std::runtime_error(fmt::format("Model::GetMesh(): could not find a mesh with number {} in \"{}\".", index, file));
}

int Model::FindBone(const std::string& name)
{
	for (auto i = 0; i < Bones.size(); i++)
	{
		if (Bones[i].Name == name)
			return i;
	}
	return NoBone;
}

void Model::CalculateBoneTransform(int id, const glm::mat4& parentTransform)
{
	auto globalTransformation = parentTransform * Bones[id].LocalTransform;

	finalBoneMatrices[id] = globalTransformation * Bones[id].Offset;
	for (auto i : Bones[id].Children)
		CalculateBoneTransform(i, globalTransformation);
}

void Model::MoveBone(int id, const glm::vec3& rotation, const glm::vec3& transform, const glm::vec3& scale)
{
	if (id == NoBone)
		return;

	auto t = glm::translate(glm::mat4(1), transform);
	auto r = (glm::mat4)glm::quat(rotation);
	auto s = glm::scale(glm::mat4(1), scale);
	Bones[id].LocalTransform = t * r * s;
}
