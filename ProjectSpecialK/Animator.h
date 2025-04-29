#pragma once

#include <memory>

class Model;

class Animator
{

public:
	std::vector<Model::Bone> Bones;

	Animator(const std::vector<Model::Bone>& bones);
	void APose();
	void CopyBones(const std::shared_ptr<Model>& client);
};

using AnimatorP = std::shared_ptr<Animator>;
