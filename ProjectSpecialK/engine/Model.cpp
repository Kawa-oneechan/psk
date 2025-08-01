﻿#include <ufbx.h>
#include <unordered_map>
#include <functional>
#include "Model.h"
#include "Utilities.h"
#include "Console.h"

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

	auto basePath = modelPath.substr(0, modelPath.rfind('/') + 1);
	auto matMapFile = modelPath.substr(0, modelPath.rfind('.')) + ".mat.json";
	auto matMap = VFS::ReadJSON(matMapFile);
	if (!matMap.is_object())
	{
		matMapFile = matMapFile.substr(0, modelPath.rfind('/')) + "/default.mat.json";
		matMap = VFS::ReadJSON(matMapFile);
		while (!matMap)
		{
			if (matMapFile.find('/') == std::string::npos)
			{
				//FatalError("No material map? How?");
				matMap = json5pp::parse5("{}");
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
		BoneCt = scene->bones.count;
		if (BoneCt >= MaxBones)
		{
			conprint(2, "Warning: {} has too many bones -- {} but can only work with {}.", modelPath, BoneCt, MaxBones);
			BoneCt = MaxBones - 1;
		}

		for (unsigned int boneIndex = 0; boneIndex < BoneCt; boneIndex++)
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
			m.Textures[0] = &fallback;
			m.Textures[1] = &fallbackNormal;
			m.Textures[2] = &white;
			m.Textures[3] = &white;

			if (node->mesh->materials.count > 0)
			{
				auto& m1 = node->mesh->materials[0]->name.data;
				bool foundIt = false;
				for (auto it : matMap.as_object())
				{
					if (it.first == m1)
					{
						auto mat = it.second.as_object();
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
							m.Textures[0] = new TextureArray(basePath + mat["albedo"].as_string());
						if (mat["normal"].is_string())
							m.Textures[1] = new TextureArray(basePath + mat["normal"].as_string());
						if (mat["mix"].is_string())
							m.Textures[2] = new TextureArray(basePath + mat["mix"].as_string());
						if (mat["opacity"].is_string())
							m.Textures[3] = new TextureArray(basePath + mat["opacity"].as_string());

						if (mat["visible"].is_boolean())
							m.Visible = mat["visible"].as_boolean();

						if (mat["translucent"].is_boolean())
							m.Translucent = mat["translucent"].as_boolean();

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
		MeshBucket::Draw(m,  pos, glm::quat(r), finalBoneMatrices, BoneCt);
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
	for (auto i = 0; i < BoneCt; i++)
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
		debprint(0, "No target model in transfer map: {}",  target->file);
		transferMap[target->Hash] = std::array<int, MaxBones>();
		map = transferMap.find(target->Hash);
		auto& m = map->second;
		m.fill(NoBone);
		auto& tb = target->Bones;
		for (int i = 0; i < this->BoneCt; i++)
		{
			auto name = this->Bones[i].Name;
			for (int j = 0; j < target->BoneCt; j++)
			{
				if (tb[j].Name == name)
				{
					debprint(1, "* {} {} --> {} {}", i, name, j, tb[j].Name);
					m[i] = j;
					break;
				}
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
