#define GLM_ENABLE_EXPERIMENTAL
#include "Camera.h"

#include "support/glm/gtc/matrix_transform.hpp"
#include "support/glm/gtx/transform.hpp"
#include "support/glm/gtx/norm.hpp"
#include "support/glm/gtx/rotate_vector.hpp"
#include "support/glm/ext/matrix_transform.hpp"
#include "support/glm/gtc/matrix_inverse.hpp"
#include "support/glm/gtx/euler_angles.hpp"
#include "support/glm/gtx/transform.hpp"

extern unsigned int commonBuffer;

Camera::Camera(glm::vec3 target, glm::vec3 angles, float distance) : _target(target), _angles(angles), _distance(distance), _swapYZ(false)
{
}

Camera::~Camera()
{
}

glm::mat4 Camera::ViewMat() const
{
	return cameraFromWorld;
}

glm::mat4 Camera::ViewMatInv() const
{
	return worldFromCamera;
}

glm::vec3 Camera::Position() const
{
	return position;
}

void Camera::Target(const glm::vec3& target)
{
	_target = target;
	_tracking = nullptr;
	Update();
}

void Camera::Angles(const glm::vec3& angles)
{
	_angles = glm::mod(angles, 360.0f);

	Update();
}

void Camera::Distance(float distance)
{
	_distance = distance;
	Update();
}

void Camera::SwapYZ(bool swapYZ)
{
	_swapYZ = swapYZ;
	Update();
}

void Camera::Set(
	const glm::vec3& target,
	const glm::vec3& angles,
	float distance
)
{
	_target = target;
	_angles = angles;
	_distance = distance;
	Update();
}

void Camera::Update()
{
	worldFromCamera = (
		glm::translate(_target)
		* (glm::eulerAngleY(glm::radians(_angles.z)))
		* (glm::eulerAngleX(glm::radians(-_angles.y)))
		* (glm::eulerAngleZ(glm::radians(_angles.x)))
		* glm::translate(glm::vec3(0, 0, _distance))
		);
	if (_swapYZ)
	{
		worldFromCamera = glm::mat4(
			1, 0, 0, 0,
			0, 0, 1, 0,
			0, -1, 0, 0,
			0, 0, 0, 1
		) * worldFromCamera;
	}
	cameraFromWorld = glm::affineInverse(worldFromCamera);
	position = worldFromCamera * glm::vec4(0, 0, 0, 1);

	glBindBuffer(GL_UNIFORM_BUFFER, commonBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, offsetof(CommonUniforms, View), sizeof(glm::mat4), &cameraFromWorld);
	glBufferSubData(GL_UNIFORM_BUFFER, offsetof(CommonUniforms, InvView), sizeof(glm::mat4), &worldFromCamera);
	int d = (int)Drum;
	glBufferSubData(GL_UNIFORM_BUFFER, offsetof(CommonUniforms, CurveEnabled), sizeof(int), &d);
	glBufferSubData(GL_UNIFORM_BUFFER, offsetof(CommonUniforms, CurveAmount), sizeof(float), &DrumAmount);
	glBufferSubData(GL_UNIFORM_BUFFER, offsetof(CommonUniforms, CurvePower), sizeof(float), &DrumPower);
}

void Camera::Target(glm::vec3* target)
{
	_tracking = target;
}

void Camera::Tick(float dt)
{
	dt;

	if (_tracking)
	{
		_target.x = _tracking->x;
		_target.z = _tracking->z;
	}

	//TODO: add tweening

	Update();
}

void Camera::Draw(float)
{
}

Camera MainCamera;
