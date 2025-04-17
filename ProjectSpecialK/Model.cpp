#include "SpecialK.h"
#include "Model.h"
#include <ufbx.h>

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
		glm::vec3 currentPos{ 0 };
		auto currentRot = glm::quat();
		unsigned int currentTextures[4]{ currentShader, currentShader, currentShader, currentShader };
		int currentLayer = -1;

		for (auto i = 0; i < meshesInBucket; i++)
		{
			bool justSwitchedShaders = false;

			auto& m = meshBucket[i];
			if (m.Shader->ID != currentShader)
			{
				justSwitchedShaders = true;
				currentShader = m.Shader->ID;
				currentLayer = -1;
				m.Shader->Use();
			}

			if (m.Position != currentPos || m.Rotation != currentRot || justSwitchedShaders)
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

	void Draw(Model::Mesh& mesh, const glm::vec3& position, const glm::quat& rotation, const glm::mat4 bones[], size_t boneCt)
	{
		meshBucket[meshesInBucket].VAO = mesh.VAO;
		meshBucket[meshesInBucket].Shader = mesh.Shader;
		meshBucket[meshesInBucket].Position = position;
		meshBucket[meshesInBucket].Rotation = rotation;
		meshBucket[meshesInBucket].Indices = mesh.Indices();
		meshBucket[meshesInBucket].BoneCount = boneCt;
		meshBucket[meshesInBucket].Layer = mesh.Layer;
		for (auto i = 0; i < 4; i++)
			meshBucket[meshesInBucket].Textures[i] = mesh.Textures[i];
		for (auto i = 0; i < boneCt; i++)
			meshBucket[meshesInBucket].Bones[i] = bones[i];

		meshesInBucket++;
		if (meshesInBucket == meshBucketSize)
			Flush();
	};
}

static std::map<std::string, std::tuple<Model*, int>> cache;
extern Shader* modelShader;
extern Shader* grassShader;
extern Shader* playerBodyShader;
extern Shader* playerEyesShader;
extern Shader* playerMouthShader;
extern Shader* playerCheekShader;
extern Shader* playerLegsShader;
extern Shader* playerHairShader;

Model::Mesh::Mesh(ufbx_mesh* mesh, std::vector<Bone>& bones) : Visible(true), Layer(0)
{
	Name = mesh->name.data;
	Hash = MatHash = GetCRC(Name);
	
	{
		auto lastUnder = Name.rfind('_');
		if (lastUnder != std::string::npos)
			MatHash = GetCRC(Name.substr(lastUnder));
	}

	Shader = modelShader; //by default
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

static glm::mat4 ufbxToGlmMat4(const ufbx_matrix& mat)
{
	glm::mat4 ret;
	for (int column = 0; column < 4; column++)
	{
		ret[column].x = (float)mat.cols[column].x;
		ret[column].y = (float)mat.cols[column].y;
		ret[column].z = (float)mat.cols[column].z;
		ret[column].w = column < 3 ? 0.0f : 1.0f;
	}
	return ret;
}

static glm::vec3 ufbxToGlmVec(const ufbx_vec3& vec)
{
	return glm::vec3(vec.x, vec.y, vec.z);
}

static glm::quat ufbxToGlmQuat(const ufbx_quat& qua)
{
	return glm::make_quat(&qua.x);
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
			if (matMapFile.find('/') == std::string::npos)
			{
				//FatalError("No material map? How?");
				matMap = JSON::Parse("{}");
				break;
			}

			matMapFile = matMapFile.substr(0, matMapFile.rfind('/'));

			auto slashes = 0;
			for (auto ch : matMapFile)
				if (ch == '/') slashes++;

			if (slashes == 0)
				matMapFile = "default.mat.json";
			else
				matMapFile = matMapFile.substr(0, matMapFile.rfind('/')) + "/default.mat.json";

			matMap = VFS::ReadJSON(matMapFile);
		}
	}

	debprint(5, "Materials:");
	for (auto m : scene->materials)
	{
		debprint(0, "* {}", m->name.data);
	}

	if (scene->bones.count > 0)
	{
		debprint(5, "Bones:");

		std::unordered_map<std::string, int> nameToBoneIndex;
		size_t numberOfBones = scene->bones.count;

		Bones.resize(numberOfBones);

		for (unsigned int boneIndex = 0; boneIndex < numberOfBones; boneIndex++)
		{
			auto& bone = *scene->bones.data[boneIndex];
			std::string boneName = bone.name.data;
			nameToBoneIndex[boneName] = boneIndex;
		}

		std::function<void(ufbx_node*, int)> traverseNodeHierarchy = [&](ufbx_node* node, int parent)
		{
			size_t numberOfChildren = node->children.count;
			int boneIndex = parent;
			std::string nodeName = node->name.data;
			bool isBone = nameToBoneIndex.find(nodeName) != nameToBoneIndex.end();

			if (isBone)
			{
				boneIndex = nameToBoneIndex[nodeName];
				auto b = Bone();
				b.Name = nodeName;
				b.Parent = parent;
				
				//Instead of _assuming_ the skin clusters are in the same order,
				//find the right skin cluster by name.
				for (int i = 0; i < scene->skin_clusters.count; i++)
				{
					std::string clusterName = scene->skin_clusters.data[i]->name.data;
					if (clusterName == b.Name)
					{
						auto& bone = scene->skin_clusters.data[i];
						b.InverseBind = ufbxToGlmMat4(bone->geometry_to_bone);
						break;
					}
				}
				//If not found, the Bone constructor will default to an Identity

				b.LocalTransform = ufbxToGlmMat4(node->node_to_parent);

				debprint(0, "* {}. {}", boneIndex, nodeName);
				for (int i = 0; i < 4; i++)
					debprint(1, u8"* │ {: .3f}, {: .3f}, {: .3f}, {: .3f} │", b.InverseBind[0][i], b.InverseBind[1][i], b.InverseBind[2][i], b.InverseBind[3][i]);

				Bones[boneIndex] = b;
			}
			for (int childIndex = 0; childIndex < numberOfChildren; childIndex++)
			{
				if (isBone)
				{
					std::string childNodeName = node->children.data[childIndex]->name.data;
					bool childIsBone = nameToBoneIndex.find(childNodeName) != nameToBoneIndex.end();
					if (childIsBone)
					{
						Bones[boneIndex].Children.push_back(nameToBoneIndex[childNodeName]);
					}
				}
				traverseNodeHierarchy(node->children.data[childIndex], boneIndex);
			}
		};
		traverseNodeHierarchy(scene->root_node, -1);

		for (int i = 0; i < Bones.size() && i < MaxBones; i++)
			finalBoneMatrices[i] = glm::mat4(1.0f);
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
			m.Textures[1] = &fallbackNormal;
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
							//TODO: use a JSON for this.
							/*
							{
								"__default": "model.fs",
								"grass": "grass.fs",
								"playerbody": "playerbody.fs"
								//etc...
							}
							Have a map of Shader objects, populated by this JSON file.
							Console reload function can use this too. Perhaps other
							stuff too.
							*/
							if (s == "grass")
								m.Shader = grassShader;
							else if (s == "playerbody")
								m.Shader = playerBodyShader;
							else if (s == "playereyes")
								m.Shader = playerEyesShader;
							else if (s == "playermouth")
								m.Shader = playerMouthShader;
							else if (s == "playercheek")
								m.Shader = playerCheekShader;
							else if (s == "playerlegs")
								m.Shader = playerLegsShader;
							else if (s == "playerhair")
								m.Shader = playerHairShader;
						}
						if (mat["albedo"])
							m.Textures[0] = new TextureArray(basePath + mat["albedo"]->AsString());
						if (mat["normal"])
							m.Textures[1] = new TextureArray(basePath + mat["normal"]->AsString());
						if (mat["mix"])
							m.Textures[2] = new TextureArray(basePath + mat["mix"]->AsString());
						if (mat["opacity"])
							m.Textures[3] = new TextureArray(basePath + mat["opacity"]->AsString());

						if (mat["visible"])
							m.Visible = mat["visible"]->AsBool();

						foundIt = true;
						break;
					}
				}
				debprint(0, "* #{}: {} > {}{}", matCt, node->name.data, m1, foundIt ? "" : " (Unmapped)");
				matCt++;
			}
			Meshes.emplace_back(m);
		}
	}

	ufbx_free_scene(scene);

	cache[file] = std::make_tuple(this, 1);
}

void Model::Draw(const glm::vec3& pos, float yaw, int mesh)
{
	int j = 0;
	for (auto& m : Meshes)
	{
		if (!m.Visible)
			continue;
		if (mesh != -1 && mesh != j)
		{
			j++;
			continue;
		}

		glm::vec3 r(0, glm::radians(yaw), 0);
		MeshBucket::Draw(m,  pos, glm::quat(r), finalBoneMatrices, Bones.size());
		j++;
	}

	glBindVertexArray(0);
}

void Model::SetVisibility(bool visible)
{
	for (auto& m : Meshes)
		m.Visible = visible;
}

void Model::SetVisibility(const std::string& name, bool visible)
{
	auto hash = GetCRC(name);
	for (auto& m : Meshes)
	{
		if (m.Hash == hash)
			m.Visible = visible;
	}
}

void Model::SetVisibility(int index, bool visible)
{
	GetMesh(index).Visible = visible;
}

void Model::SetLayer(int layer)
{
	for (auto& m : Meshes)
		m.Layer = layer;
}

void Model::SetLayer(const std::string& name, int layer)
{
	auto hash = GetCRC(name);
	for (auto& m : Meshes)
	{
		if (m.Hash == hash)
			m.Layer = layer;
	}
}

void Model::SetLayerByMat(const std::string& name, int layer)
{
	auto hash = GetCRC(name);
	for (auto& m : Meshes)
	{
		if (m.MatHash == hash)
			m.Layer = layer;
	}
}

void Model::SetLayer(int index, int layer)
{
	GetMesh(index).Layer = layer;
}

Model::Mesh& Model::GetMesh(const std::string& name)
{
	auto hash = GetCRC(name);
	for (auto& m : Meshes)
	{
		if (m.Hash == hash || m.MatHash == hash)
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

void Model::CalculateBoneTransform(int id)
{
	auto& bone = Bones[id];
	if (bone.Parent != NoBone)
		bone.GlobalTransform = Bones[bone.Parent].GlobalTransform * bone.LocalTransform * bone.GetAnimTransform();
	else
		bone.GlobalTransform = bone.LocalTransform * bone.GetAnimTransform();

	for (auto i : Bones[id].Children)
		CalculateBoneTransform(i);
}

void Model::CalculateBoneTransforms()
{
	//Way I load my armature, the root can be *any* ID. Find it.
	auto root = FindBone("Root");
	if (root == NoBone)
		root = FindBone("Mdl_Root");
	if (root == NoBone)
		root = FindBone("Skl_Root");
	if (root == NoBone)
		return; //give up for now

	CalculateBoneTransform(root);

	//Bring back into model space
	for (int i = 0; i < Bones.size(); i++)
		finalBoneMatrices[i] = Bones[i].GlobalTransform * Bones[i].InverseBind;
}
