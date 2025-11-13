#include "Model.h"
#include "Shader.h"
#include "Texture.h"

#ifndef BECKETT_NO3DMODELS

extern unsigned int currentVAO;
extern bool wireframe;

__declspec(noreturn)
extern void FatalError(const std::string& message);

namespace MeshBucket
{
	static constexpr int meshBucketSize = 64;
	static constexpr int transMeshBucketSize = 256;

	bool DepthOnlyPass = false;

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
		bool Billboard;
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
		unsigned int currentTextures[4]{ (unsigned int)-1, (unsigned int)-1, (unsigned int)-1, (unsigned int)-1 };
		int currentLayer = -1;

		Shader* theShader = Shaders["model"];

		for (auto i = 0; i < meshesInBucket; i++)
		{
			bool justSwitchedShaders = false;

			auto& m = meshBucket[i];
			if (m.Shader->ID != currentShader)
			{
				justSwitchedShaders = true;
				theShader = m.Shader;
				currentShader = theShader->ID;
				currentLayer = -1;
				theShader->Use();
			}

			if (m.Position != currentPos || m.Rotation != currentRot || justSwitchedShaders)
			{
				currentPos = m.Position;
				currentRot = m.Rotation;

				auto t = glm::translate(glm::mat4(1), m.Position);
				auto r = (glm::mat4)m.Rotation;
				//auto s = glm::scale(glm::mat4(1), scale);
				auto model = t * r; //* s;
				if (m.Billboard)
					model[0][3] = 1.0;

				theShader->Set("model", model);
			}

			theShader->Set("finalBonesMatrices", m.Bones[0], m.BoneCount);

			for (auto j = DepthOnlyPass ? 3 : 0; j < 4; j++)
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
				theShader->Set("layer", m.Layer);
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
		if (DepthOnlyPass)
			return; //don't render translucents in the depth prepass

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
		bucket.Billboard = mesh.Billboard;
		for (auto i = 0; i < 4; i++)
			bucket.Textures[i] = mesh.Textures[i];
		for (auto i = 0; i < boneCt; i++)
			bucket.Bones[i] = bones[i];

		if (DepthOnlyPass)
		{
			bucket.Shader = mesh.Opaque ? Shaders["depthpass2"] : Shaders["depthpass"];
		}
		if (!mesh.Translucent)
		{
			meshesInBucket++;
			if (meshesInBucket == meshBucketSize)
				Flush();
		}
		else if (!DepthOnlyPass)
		{
			transMeshBucket[transMeshesInBucket] = meshBucket[meshesInBucket];
			bucket.VAO = 0;
			transMeshesInBucket++;
			if (transMeshesInBucket == transMeshBucketSize)
				FlushTranslucent();
		}
	};

	void DrawAllWithDepth(float dt, const std::function<void(void)>& renderer)
	{
		if (Shaders.count("depthpass") == 0 || Shaders.count("depthpass2") == 0) //-V838
			FatalError("Cannot do depth prepass rendering without \"depthpass\" and \"depthpass2\" shaders.");

		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Depth pass");

		DepthOnlyPass = true;
		glColorMask(0, 0, 0, 0);
		glDepthMask(GL_TRUE);
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		renderer();

		glPopDebugGroup();
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Color pass");

		DepthOnlyPass = false;
		glDepthFunc(GL_LEQUAL);
		glColorMask(1, 1, 1, 1);
		glDepthMask(GL_FALSE);
		glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);

		renderer();

		glPopDebugGroup();

		glDisable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

#endif
