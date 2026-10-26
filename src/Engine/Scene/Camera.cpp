#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

Camera::Camera()
{
	UpdateDirection();
}

void Camera::Move(float forward, float right, float deltaTime)
{
	// 根据当前朝向计算相机的右方向
	const glm::vec3 rightDirection = glm::normalize(glm::cross(m_Front, m_WorldUp));

	// 合并前后、左右的移动输入
	const glm::vec3 movement = m_Front * forward + rightDirection * right;

	if (glm::length(movement) > 0.0f)
	{
		// 归一化，避免斜向移动更快
		m_Position += glm::normalize(movement) * m_MoveSpeed * deltaTime;
	}
}

void Camera::Rotate(float offsetX, float offsetY)
{
	m_Yaw += offsetX * m_MouseSensitivity;
	m_Pitch += offsetY * m_MouseSensitivity;

	m_Pitch = glm::clamp(m_Pitch, -89.0f, 89.0f);

	UpdateDirection();
}

glm::mat4 Camera::GetViewMatrix() const
{
	return glm::lookAt(
		m_Position,
		m_Position + m_Front,
		m_WorldUp
	);
}

glm::vec3 Camera::GetPosition() const
{
	return m_Position;
}

void Camera::UpdateDirection()
{
	const float yawRadians = glm::radians(m_Yaw);
	const float pitchRadians = glm::radians(m_Pitch);

	// 单位球体
	// 不考虑Y轴的情况下，向量在XZ平面
	// 则 x分量(邻边)/半径1 = sin theta
	// z分量(对边)/半径1 = cos theta

	// 考虑Y轴的情况下，则 y分量(对边)/半径1 = sin alpha
	// Y轴在XZ屏幕的投影是cos alpha
	// 把这个实际长度代入声明半径1之中，可以得到下面式子
	m_Front = glm::normalize(glm::vec3(
		std::cos(yawRadians) * std::cos(pitchRadians),
		std::sin(pitchRadians),
		std::sin(yawRadians) * std::cos(pitchRadians)
	));
}