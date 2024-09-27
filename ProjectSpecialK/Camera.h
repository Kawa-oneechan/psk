#pragma once

#include "support/glm/glm.hpp"
//#include "support/glm/gtc/matrix_transform.hpp"

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	glm::vec3 Target;

	bool Free = true;

	float Yaw;
	float Pitch;

	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0, float pitch = 0.0f);
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

	void Setup(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0, float pitch = 0.0f);

	glm::mat4 GetViewMatrix();
	void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
	void ProcessMouseScroll(float yoffset);

	void UpdateCameraVectors();
};

extern Camera MainCamera;
