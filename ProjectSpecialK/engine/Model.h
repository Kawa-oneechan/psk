#pragma once
#include <array>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>
#include <format.h>
#include "Types.h"
#include "Shader.h"
#include "Texture.h"
#include "VFS.h"

//Because void-casting an integer for legacy reasons is silly.

inline void kawa_glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLsizeiptr offset)
{
	glad_glVertexAttribPointer(index, size, type, normalized, stride, (void*)offset);
}
inline void kawa_glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, GLsizeiptr offset)
{
	glad_glVertexAttribIPointer(index, size, type, stride, (void*)offset);
}
#undef glVertexAttribPointer
#undef glVertexAttribIPointer
#define glVertexAttribPointer kawa_glVertexAttribPointer
#define glVertexAttribIPointer kawa_glVertexAttribIPointer

//Max amount of bones in a mesh
static constexpr int MaxBones = 100;

struct ufbx_mesh;

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
		bool Visible;
		hash Hash, MatHash;
		std::string Name;
		Shader* Shader;
		int Layer;
		bool Translucent;

		Mesh(ufbx_mesh* mesh, std::array<Bone, MaxBones>& bones, size_t boneCt);
		const size_t Indices() { return indices.size(); }
	};

private:
	std::string file;
	TextureArray fallback{ TextureArray("fallback.png") };
	TextureArray fallbackNormal{ TextureArray("fallback_nrm.png") };
	TextureArray white{ TextureArray("white.png") };

	void CalculateBoneTransform(int id);

public:
	std::vector<Mesh> Meshes;
	std::array<Model::Bone, MaxBones> Bones;
	glm::mat4 finalBoneMatrices[MaxBones];
	size_t BoneCt;

	hash Hash;

	const bool IsSkinned() const { return Bones.size() > 0; }

	Model() = default;
	Model(const std::string& modelPath);

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
}

using ModelP = std::shared_ptr<Model>;
using Armature = std::array<Model::Bone, MaxBones>;
