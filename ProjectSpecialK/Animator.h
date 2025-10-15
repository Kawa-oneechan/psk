#pragma once

class Model;
using Armature = std::array<Model::Bone, MaxBones>;

class Animator
{

public:
	Armature Bones;

	explicit Animator(const Armature& bones);
	void APose();
	void CopyBones(const std::shared_ptr<Model>& client);
};

using AnimatorP = std::shared_ptr<Animator>;
