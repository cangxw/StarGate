#define GLFW_INCLUDE_NONE

#include "EditorInput.h"
#include "Engine/Camera.h"
#include "Engine/SceneData.h"
#include "EditorUI.h"

#include <GLFW/glfw3.h>

void EditorInput::InitializeMouse(GLFWwindow* window, Camera& camera)
{
	m_MouseState = MouseState{};
	m_MouseState.camera = &camera;

	glfwSetWindowUserPointer(window, &m_MouseState);

	// 隐藏光标
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window,
		[](GLFWwindow* window, double xpos, double ypos)
		{
			// 取回相机输入状态
			MouseState* input = static_cast<MouseState*>(glfwGetWindowUserPointer(window));

			if (input == nullptr || input->camera == nullptr)
			{
				return;
			}

			if (!input->cameraControlActive)
			{
				//没有控制相机的时候，重置
				input->firstMouse = true;
				return;
			}

			if (input->firstMouse)
			{
				input->lastMouseX = xpos;
				input->lastMouseY = ypos;
				input->firstMouse = false;
				return;
			}

			const float offsetX = static_cast<float>(xpos - input->lastMouseX);
			const float offsetY = static_cast<float>(input->lastMouseY - ypos);

			input->lastMouseX = xpos;
			input->lastMouseY = ypos;

			input->camera->Rotate(offsetX, offsetY);
		}
	);
}

void EditorInput::UpdateCameraMouse(GLFWwindow* window, const SceneViewState& sceneView)
{
	// 鼠标的处理
	const bool windowFocused = glfwGetWindowAttrib(window, GLFW_FOCUSED) == GLFW_TRUE;
	const bool rightMouseDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

	//在Scene场景里按下才算开始控制相机
	const bool startCameraControl = sceneView.hovered && sceneView.rightMouseClicked;

	// 已经开始控制后，保持到松开右键
	const bool enableCamera =
		windowFocused &&
		sceneView.imageVisible &&
		rightMouseDown &&
		(m_MouseState.cameraControlActive || startCameraControl);

	if (enableCamera != m_MouseState.cameraControlActive)
	{
		m_MouseState.cameraControlActive = enableCamera;
		m_MouseState.firstMouse = true;

		glfwSetInputMode(window, GLFW_CURSOR,
			enableCamera ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL
		);
	}
}

void EditorInput::ShutdownMouse(GLFWwindow* window)
{
	glfwSetCursorPosCallback(window, nullptr);
	glfwSetWindowUserPointer(window, nullptr); // 清除窗口的用户指针，避免悬空指针

	m_MouseState = MouseState{};
}


void EditorInput::UpdateObjectKeyboard(GLFWwindow* window, SceneData& scene, std::size_t selectObjectIndex, float deltaTime)
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

void EditorInput::UpdateExitKeyboard(GLFWwindow* window)
{
	// 输入处理
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}
bool EditorInput::DeleteSelectedObject(SceneData& scene, std::size_t& selectedObjectIndex)
{
	if (selectedObjectIndex >= scene.objects.size())
	{
		return false;
	}

	scene.objects.erase(scene.objects.begin() + static_cast<std::ptrdiff_t>(selectedObjectIndex));

	// 删除最后一个物体后，暂时将索引保持为 0；
	// 后续必须检查是否存在选中物体
	if (scene.objects.empty())
	{
		selectedObjectIndex = 0;
	}
	else if (selectedObjectIndex >= scene.objects.size())
	{
		selectedObjectIndex = scene.objects.size() - 1;
	}

	return true;
}