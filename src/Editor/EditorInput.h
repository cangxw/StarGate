#pragma once
#include <cstddef>
#include <glm/glm.hpp>
struct GLFWwindow;
class Camera;
struct SceneData;
struct SceneViewState;

class EditorInput
{
public:
	EditorInput() = default;
	EditorInput(const EditorInput&) = delete;
	EditorInput& operator=(const EditorInput&) = delete;

	void InitializeMouse(GLFWwindow* window, Camera& camera);
	void UpdateCameraMouse(GLFWwindow* window, const SceneViewState& sceneView);
	void ShutdownMouse(GLFWwindow* window);
	
	void UpdateCameraKeyboard(GLFWwindow* window, Camera& camera, float deltaTime);
	void UpdateObjectKeyboard(GLFWwindow* window, SceneData& scene, std::size_t selectObjectIndex, float deltaTime);
	void UpdateLightKeyboard(GLFWwindow* window, glm::vec3& lightColor);
	void UpdateExitKeyboard(GLFWwindow* window);
	bool DeleteSelectedObject(SceneData&  scene, std::size_t& selectedObjectIndex);
private:
	float m_ObjectMoveSpeed = 0.8f;
	float m_ObjectRotationSpeed = 90.0f;

	struct MouseState
	{
		Camera* camera = nullptr;

		double lastMouseX = 0.0;
		double lastMouseY = 0.0;

		bool firstMouse = true;
		bool cameraControlActive = false;
	};

	MouseState m_MouseState;
};