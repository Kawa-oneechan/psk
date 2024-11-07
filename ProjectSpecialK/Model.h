#pragma once

#include "support/glad/glad.h"
#include "support/stb_image.h"
#include "support/format.h"
#include "support/ufbx.h"
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

class Model
{
	//Max amount of bones in a mesh
	static constexpr int MaxBones = 50;
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

	struct Bone
	{
		std::string Name;
		glm::mat4 Offset;
		glm::mat4 LocalTransform{ glm::mat4(1) };
		std::vector<int> Children;
	};

	class Mesh
	{
	private:
		unsigned int VBO, VAO, EBO;
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
	public:
		int Texture;
		bool Visible;
		hash Hash;
		std::string Name;

		Mesh(ufbx_mesh* mesh, std::vector<Bone>& bones);
		const void Draw();
	};

private:
	std::string file;
	TextureArray fallback{ TextureArray("fallback.png") };
	TextureArray white{ TextureArray("white.png") };

public:
	std::vector<Mesh> Meshes;
	std::array<Texture*, 32> Textures;
	std::array<int, 4> TexArrayLayers;
	std::vector<Bone> Bones;
	glm::mat4 finalBoneMatrices[Model::MaxBones];

	const bool IsSkinned() const { return Bones.size() > 0; }

	Model() = default;
	Model(const std::string& modelPath);

	void Draw(Shader* shader, const glm::vec3& pos = glm::vec3(0), float yaw = 0);
	void AllVisible();
	Mesh& GetMesh(const std::string& name);

	int FindBone(const std::string& name);
	void CalculateBoneTransform(int id, const glm::mat4& parentTransform = glm::mat4(1.0f));
	void MoveBone(int id, float deg, const glm::vec3& rotation, const glm::vec3& transform = glm::vec3(0));
};

using ModelP = std::shared_ptr<Model>;
