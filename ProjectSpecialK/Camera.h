#pragma once

#include "SpecialK.h"

class Camera : public Tickable
{
protected:
	glm::vec3 _target{};
	glm::vec3 _angles{};
	glm::vec3 _offset{};
	float _distance{ 0 };

	glm::vec3* _tracking{};

	glm::vec3 position{};

	bool _swapYZ;

public:
	Camera(const Camera& c) = default;
	Camera(glm::vec3 target = glm::vec3(0), glm::vec3 angles = glm::vec3(0), float distance = 10);
	virtual ~Camera();

	glm::vec3 Position() const;
	inline const glm::vec3& Target() const { return _target; }
	inline const glm::vec3& Angles() const { return _angles; }
	inline const glm::vec3& Offset() const { return _offset; }
	inline float Distance() const { return _distance; }
	inline bool SwapYZ() const { return _swapYZ; }

	void Target(const glm::vec3& target);
	void Angles(const glm::vec3& angles);
	void Offset(const glm::vec3& offset);
	void Distance(float distance);
	void SwapYZ(bool swapYZ);
	void Set(
		const glm::vec3& target,
		const glm::vec3& angles,
		const glm::vec3& offset,
		float distance
	);

	inline glm::vec3& GetTarget() { return _target; }
	inline glm::vec3& GetAngles() { return _angles; }
	inline glm::vec3& GetOffset() { return _offset; }
	inline float& GetDistance() { return _distance; }
	inline bool& GetSwapYZ() { return _swapYZ; }

	void Update();

	bool Locked = false;
	
	void Target(glm::vec3* target);

	void Tick(float dt);
	void Draw(float dt);
};

extern Camera MainCamera;
