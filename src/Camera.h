#pragma once

#include <glm/glm.hpp>

class Camera
{
public:
	Camera();

	void Move(float forward, float right, float deltaTime);
	void Rotate(float offsetX, float offsetY);

	// 获取视图矩阵
	glm::mat4 GetViewMatrix() const;
private:
	void UpdateDirection();

	glm::vec3 m_Position{ 0.0f, 0.0f, 3.0f };
	glm::vec3 m_Front{ 0.0f, 0.0f, -1.0f };
	glm::vec3 m_WorldUp{ 0.0f, 1.0f, 0.0f };

	float m_Yaw = -90.0f;
	float m_Pitch = 0.0f;

	float m_MoveSpeed = 2.5f;
	float m_MouseSensitivity = 0.1f;
};