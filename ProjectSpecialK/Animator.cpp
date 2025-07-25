#include "SpecialK.h"
#include "Animator.h"
#include "engine/Model.h"

Animator::Animator(const Armature& clientBones)
{
	/*
	Bones.reserve(clientBones.size());
	for (auto b : clientBones)
		Bones.push_back(b);
	*/
	std::copy(clientBones.begin(), clientBones.end(), Bones.begin());
}

static int findBone(const Armature& bones, const std::string& name)
{
	for (auto i = 0; i < bones.size(); i++)
	{
		if (bones[i].Name == name)
			return i;
	}
	return -1;
}

void Animator::APose()
{
	for (auto& bone : Bones)
		bone.Rotation = glm::vec3(0.0f);
	
	auto apply = [&](const std::string& bone, const glm::vec3& rot)
	{
		auto b = findBone(Bones, bone);
		if (b == -1) return;
		Bones[b].Rotation = rot;
	};

	apply("Arm_1_L", glm::vec3(0, -1.2f, -0.4f));
	apply("Arm_2_L", glm::vec3(0, 0, 0.5f));
	apply("Arm_1_R", glm::vec3(0, -1.2f, -0.4f));
	apply("Arm_2_R", glm::vec3(0, 0, 0.5f));
}

void Animator::CopyBones(const std::shared_ptr<Model>& client)
{
	//We assume that the client's bones are in the same order as ours.
	auto& cbones = client->Bones;
	for (auto i = 0; i < client->BoneCt; i++)
	{
		cbones[i].Rotation = Bones[i].Rotation;
		//translation?
		//scale?
	}
}
