#include <unordered_map>
#include <functional>
#include <algorithm>
#include <ufbx.h>
#include "Model.h"
#include "Utilities.h"
#include "Console.h"
#include "Shader.h"
#include "Texture.h"

#ifndef BECKETT_NO3DMODELS

__declspec(noreturn)
	extern void FatalError(const std::string& message);

static std::map<std::string, std::tuple<Model*, int>> cache;
static std::map<hash, std::map<hash, std::array<int, MaxBones>>> transferMaps;

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

#pragma warning(push)
#pragma warning(disable: 4505)
//Keeping these for later
static glm::vec3 ufbxToGlmVec(const ufbx_vec3& vec)
{
	return glm::vec3(vec.x, vec.y, vec.z);
}

static glm::quat ufbxToGlmQuat(const ufbx_quat& qua)
{
	return glm::make_quat(&qua.x);
}
#pragma warning(pop)

Model::Model(const std::string& modelPath) : file(modelPath)
{
	auto c = cache.find(modelPath);
	if (c != cache.end())
	{
		Model* m;
		int r;
		std::tie(m, r) = c->second;
		Meshes = m->Meshes;
		Hash = m->Hash;
		r++;
		cache[file] = std::make_tuple(m, r);
		return;
	}

	Hash = GetCRC(file);

	size_t vfsSize = 0;
	auto vfsData = VFS::ReadData(modelPath, &vfsSize);

	ufbx_load_opts options = {};
	ufbx_error errors;
	ufbx_scene *scene = ufbx_load_memory(vfsData.get(), vfsSize, &options, &errors);
	if (!scene)
		FatalError(fmt::format("Could not load scene {}: {}", modelPath, errors.description.data));

	debprint(5, "Loading {}\n-------------------------------", modelPath);

	auto basePath = VFS::GetPathPart(modelPath);
	auto matMap = VFS::ReadJSON(VFS::ClimbDown(VFS::ChangeExtension(modelPath, "mat.json"), "default.mat.json"));
	if (!matMap.is_object())
		matMap = json5pp::parse5("{}");

	debprint(5, "Materials:");
	for (auto m : scene->materials)
	{
		debprint(0, "* {}", m->name.data);
	}

	if (scene->bones.count > 0)
	{
		debprint(5, "Bones:");

		std::unordered_map<std::string, int> nameToBoneIndex;
		BoneCt = scene->bones.count;
		if (BoneCt >= MaxBones)
		{
			conprint(2, "Warning: {} has too many bones -- {} but can only work with {}.", modelPath, BoneCt, MaxBones);
			BoneCt = MaxBones - 1;
		}

		for (unsigned int boneIndex = 0; boneIndex < BoneCt; boneIndex++)
		{
			const auto& bone = *scene->bones.data[boneIndex];
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
						const auto& bone = scene->skin_clusters.data[i];
						b.InverseBind = ufbxToGlmMat4(bone->geometry_to_bone);
						break;
					}
				}
				//If not found, the Bone constructor will default to an Identity

				b.LocalTransform = ufbxToGlmMat4(node->node_to_parent);

				debprint(0, "* {}. {}", boneIndex, nodeName);

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

		for (int i = 0; i < BoneCt && i < MaxBones; i++)
			finalBoneMatrices[i] = glm::mat4(1.0f);
	}

	debprint(5, "Meshes:");
	unsigned int matCt = 0;
	for (size_t i = 0; i < scene->nodes.count; i++)
	{
		ufbx_node *node = scene->nodes.data[i];
		if (node->mesh)
		{
			auto m = Mesh(node->mesh, Bones, BoneCt);
			m.Shader = Shaders["model"];
			m.Translucent = false;
			m.Opaque = false;
			m.Textures[0] = &fallback;
			m.Textures[1] = &fallbackNormal;
			m.Textures[2] = &white;
			m.Textures[3] = &white;

			if (node->mesh->materials.count > 0)
			{
				const auto& m1 = node->mesh->materials[0]->name.data;
				auto mmObj = matMap.as_object();
				auto it = std::find_if(mmObj.cbegin(), mmObj.cend(), [m1](auto e)
				{
					return e.first == m1;
				});
				if (it != mmObj.cend())
				{
					auto mat = it->second.as_object();
					if (mat["shader"])
					{
						auto s = mat["shader"].as_string();
						m.Shader = Shaders[s];
						/*
						Secondary idea: look into custom properties in the FBX file
						that may OVERRIDE the matmap file.
						*/
					}
					if (mat["albedo"].is_string())
						m.Textures[0] = new TextureArray(fmt::format("{}/{}", basePath, mat["albedo"].as_string()));
					if (mat["normal"].is_string())
						m.Textures[1] = new TextureArray(fmt::format("{}/{}", basePath, mat["normal"].as_string()));
					if (mat["mix"].is_string())
						m.Textures[2] = new TextureArray(fmt::format("{}/{}", basePath, mat["mix"].as_string()));
					if (mat["opacity"].is_string())
						m.Textures[3] = new TextureArray(fmt::format("{}/{}", basePath, mat["opacity"].as_string()));

					if (mat["visible"].is_boolean())
						m.Visible = mat["visible"].as_boolean();

					if (mat["translucent"].is_boolean())
						m.Translucent = mat["translucent"].as_boolean();
					if (mat["opaque"].is_boolean())
						m.Opaque = mat["opaque"].as_boolean();
					if (mat["billboard"].is_boolean())
						m.Billboard = mat["billboard"].as_boolean();

					if (mat["nearest"].is_boolean() && mat["nearest"].as_boolean())
					{
						m.Textures[0]->SetFilter(GL_NEAREST);
						m.Textures[1]->SetFilter(GL_NEAREST);
						m.Textures[2]->SetFilter(GL_NEAREST);
						m.Textures[3]->SetFilter(GL_NEAREST);
					}
					debprint(0, "* #{}: {} > {}", matCt, node->name.data, m1);
				}
				else
					debprint(0, "* #{}: {} > {} (unmapped)", matCt, node->name.data, m1);
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
		MeshBucket::Draw(m, pos, glm::quat(r), finalBoneMatrices, BoneCt);
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
	auto it = std::find_if(Meshes.cbegin(), Meshes.cend(), [hash](const auto& e)
	{
		return e.Hash == hash || e.MatHash == hash;
	});
	if (it != Meshes.cend())
		return (Mesh&)(*it);
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
	auto it = std::find_if(Bones.cbegin(), Bones.cend(), [name](const auto& e)
	{
		return e.Name == name;
	});
	if (it != Bones.cend())
		return (int)(it - Bones.cbegin());
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
	for (int i = 0; i < BoneCt; i++)
		finalBoneMatrices[i] = Bones[i].GlobalTransform * Bones[i].InverseBind;
}

void Model::CopyBoneTransforms(ModelP target)
{
	if (!target)
		return;

	auto me = transferMaps.find(this->Hash);
	if (me == transferMaps.end())
	{
		debprint(0, "No source model in transfer map: {}", this->file);
		transferMaps[this->Hash] = std::map<hash, std::array<int, MaxBones>>();
		me = transferMaps.find(this->Hash);
	}
	auto& transferMap = me->second;

	auto map = transferMap.find(target->Hash);
	if (map == transferMap.end())
	{
		debprint(0, "No target model in transfer map: {}", target->file);
		transferMap[target->Hash] = std::array<int, MaxBones>();
		map = transferMap.find(target->Hash);
		auto& m = map->second;
		m.fill(NoBone);
		auto& tb = target->Bones;

		for (int i = 0; i < this->BoneCt; i++)
		{
			auto name = this->Bones[i].Name;
			auto end = tb.cbegin() + target->BoneCt;
			auto it = std::find_if(tb.cbegin(), end, [name](const auto& e)
			{
				return e.Name == name;
			});
			if (it != end)
			{
				auto j = (int)(it - tb.cbegin());
				debprint(1, "* {} {} --> {} {}", i, name, j, tb[j].Name);
				m[i] = j;
			}
		}
	}
	auto& m = map->second;

	for (int i = 0; i < BoneCt; i++)
	{
		if (m[i] == NoBone)
			continue;
		auto& sb = this->Bones[i];
		auto& tb = target->Bones[m[i]];
		tb.Rotation = sb.Rotation;
		tb.Translation = sb.Translation;
		tb.Scale = sb.Scale;
	}

	target->CalculateBoneTransforms();
}

#endif
