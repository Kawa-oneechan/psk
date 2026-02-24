#pragma once
#include <array>
#include <functional>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>
#include <format.h>
#include "Types.h"
#include "Texture.h"
#include "VFS.h"
#include "../Game.h"

//Because void-casting an integer for legacy reasons is silly.

inline void kawa_glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLsizeiptr offset)
{
glad_glVertexAttribPointer(index, size, type, normalized, stride, reinterpret_cast<void*>(offset));
}
inline void kawa_glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, GLsizeiptr offset)
{
glad_glVertexAttribIPointer(index, size, type, stride, reinterpret_cast<void*>(offset));
}
#undef glVertexAttribPointer
#undef glVertexAttribIPointer
#define glVertexAttribPointer kawa_glVertexAttribPointer
#define glVertexAttribIPointer kawa_glVertexAttribIPointer

#ifndef BECKETT_NO3DMODELS

struct ufbx_mesh;
class Shader;

//Max amount of bones in a mesh
static constexpr int MaxBones = 100;

class Model
{
	//Max amount of bones and weights per vertex
	static constexpr int MaxWeights = 4;
	//No bone assigned or found
	static constexpr int NoBone = -1;

	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
		glm::vec3 Tangent;
		int Bones[Model::MaxWeights];
		float Weights[Model::MaxWeights];
	};

public:
	struct Bone
	{
		std::string Name;
		glm::mat4 InverseBind{ glm::mat4(1) };
		glm::mat4 LocalTransform{ glm::mat4(1) };
		glm::mat4 GlobalTransform{ glm::mat4(1) };
		std::vector<int> Children;
		int Parent{ NoBone };

		glm::vec3 Translation{ glm::vec3(0) };
		glm::vec3 Rotation{ glm::vec3(0) };
		glm::vec3 Scale{ glm::vec3(1) };

		glm::mat4 GetAnimTransform()
		{
			return
				glm::translate(glm::mat4(1.0f), Translation) *
				glm::mat4(glm::quat(Rotation)) *
				glm::scale(glm::mat4(1.0f), Scale);
		}
	};

	class Mesh
	{
	private:
		unsigned int VBO, EBO;
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
	public:
		unsigned int VAO;
		TextureArray* Textures[4];
		std::string Name;
		Shader* Shader;
		hash Hash, MatHash;
		bool Visible;
		int Layer;
		bool Translucent;
		bool Opaque;
		bool Billboard{ false };

		Mesh(ufbx_mesh* mesh, const std::array<Bone, MaxBones>& bones, size_t boneCt);
		const size_t Indices() const { return indices.size(); }
	};

private:
	std::string file;

	void CalculateBoneTransform(int id);

public:
	std::vector<Mesh> Meshes;
	std::array<Model::Bone, MaxBones> Bones;
	glm::mat4 finalBoneMatrices[MaxBones];
	size_t BoneCt{ 0 };

	hash Hash{ 0 };

	const bool IsSkinned() const { return BoneCt > 0; }

	Model() = default;
	explicit Model(const std::string& modelPath);
#if 0
	~Model(); 
#endif

	//Queues the model for drawing at the specified position and rotation. If the mesh argument is -1, the entire model is drawn.
	void Draw(const glm::vec3& pos = glm::vec3(0), float yaw = 0, int mesh = -1);
	//Sets the visibility of all parts of the model.
	void SetVisibility(bool visible = true);
	//Sets the visibility of a given mesh part, specified by mesh name.
	void SetVisibility(const std::string& name, bool visible = true);
	//Sets the visibility of a given mesh part, specified by index.
	void SetVisibility(int index, bool visible = true);
	//Sets the TextureArray layer index for all parts of the model.
	void SetLayer(int layer = 0);
	//Sets the TextureArray layer index of a given mesh part, specified by mesh name.
	void SetLayer(const std::string& name, int layer);
	//Sets the TextureArray layer index of a given mesh part, specified by material name.
	void SetLayerByMat(const std::string& name, int layer);
	//Sets the TextureArray layer index of a given mesh part, specified by index.
	void SetLayer(int index, int layer);

	//Returns a reference to a Mesh by name, which can be either the full mesh or the material part.
	Mesh& GetMesh(const std::string& name);
	//Returns a reference to a Mesh by index.
	Mesh& GetMesh(int index);

	//Returns the index of a bone for this model by name.
	int FindBone(const std::string& name);
	//Calcalutes the final bone matrices for the entire armature.
	void CalculateBoneTransforms();

	void CopyBoneTransforms(std::shared_ptr<Model> target);
};

namespace MeshBucket
{
	extern void Flush();
	extern void FlushTranslucent();
	extern void Draw(Model::Mesh& mesh, const glm::vec3& position, const glm::quat& rotation, const glm::mat4 bones[], size_t boneCt);
	extern void DrawAllWithDepth(float dt, const std::function<void(void)>& renderer);
}

using ModelP = std::shared_ptr<Model>;
using Armature = std::array<Model::Bone, MaxBones>;

class UfbxMisc
{
public:
	struct Light
	{
		glm::vec3 Position;
		glm::vec4 Color;
	};
	struct Camera
	{
		glm::vec3 Position;
		glm::vec3 Direction;
	};
	std::vector<Light> Lights;
	std::vector<Camera> Cameras;

	explicit UfbxMisc(const std::string& modelPath);
};

#endif
