#include "Model.h"

unsigned int currentVAO = 0;

namespace MeshBucket
{
	static constexpr int meshBucketSize = 64;
	static constexpr int transMeshBucketSize = 256;

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

	static int transMeshesInBucket;
	static std::array<MeshInABucket, transMeshBucketSize> transMeshBucket;

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

	void FlushTranslucent()
	{
		for (auto i = 0; i < transMeshesInBucket; i++)
		{
			meshBucket[meshesInBucket] = transMeshBucket[i];
			meshesInBucket++;
			if (meshesInBucket == meshBucketSize)
				Flush();
		}
		transMeshesInBucket = 0;
		Flush();
	}

	void Draw(Model::Mesh& mesh, const glm::vec3& position, const glm::quat& rotation, const glm::mat4 bones[], size_t boneCt)
	{
		auto& bucket = meshBucket[meshesInBucket];

		bucket.VAO = mesh.VAO;
		bucket.Shader = mesh.Shader;
		bucket.Position = position;
		bucket.Rotation = rotation;
		bucket.Indices = mesh.Indices();
		bucket.BoneCount = boneCt;
		bucket.Layer = mesh.Layer;
		for (auto i = 0; i < 4; i++)
			bucket.Textures[i] = mesh.Textures[i];
		for (auto i = 0; i < boneCt; i++)
			bucket.Bones[i] = bones[i];

		if (!mesh.Translucent)
		{
			meshesInBucket++;
			if (meshesInBucket == meshBucketSize)
				Flush();
		}
		else
		{
			transMeshBucket[transMeshesInBucket] = meshBucket[meshesInBucket];
			bucket.VAO = 0;
			transMeshesInBucket++;
			if (transMeshesInBucket == transMeshBucketSize)
				FlushTranslucent();
		}
	};
}
