#define GLFW_INCLUDE_NONE

#include "EditorInput.h"
#include "Camera.h"
#include "SceneData.h"

#include <GLFW/glfw3.h>

void EditorInput::UpdateObjectKeyboard(GLFWwindow * window, SceneData & scene, std::size_t selectObjectIndex, float deltaTime)
{
	if (selectObjectIndex >= scene.objects.size())
		return;

	SceneObject& ketboardObject = scene.objects[selectObjectIndex];
	const float moveDistance = m_ObjectMoveSpeed * deltaTime;
	const float rotationStep = m_ObjectRotationSpeed * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		ketboardObject.position.x -= moveDistance;
	}

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		ketboardObject.position.x += moveDistance;
	}

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		ketboardObject.rotation.y += rotationStep; // 逆时针旋转
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		ketboardObject.rotation.y -= rotationStep; // 顺时针旋转
	}
}

void EditorInput::UpdateLightKeyboard(GLFWwindow* window, glm::vec3& lightColor)
{
	// 切换灯光颜色
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
	{
		lightColor = glm::vec3(1.0f, 1.0f, 1.0f); // 白光
	}
	else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
	{
		lightColor = glm::vec3(1.0f, 0.8f, 0.5f); // 暖色
	}
	else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
	{
		lightColor = glm::vec3(0.4f, 0.7f, 1.0f); // 冷色
	}
}

void EditorInput::UpdateCameraKeyboard(GLFWwindow* window, Camera& camera, float deltaTime)
{
	float forward = 0.0f;
	float right = 0.0f;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		forward += 1.0f;
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		forward -= 1.0f;
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		right -= 1.0f;
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		right += 1.0f;
	}

	camera.Move(forward, right, static_cast<float>(deltaTime));
}