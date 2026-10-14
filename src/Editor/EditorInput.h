#pragma once
#include <cstddef>
#include <glm/glm.hpp>
struct GLFWwindow;
class Camera;
struct SceneData;


class EditorInput
{
public:
	void UpdateCameraKeyboard(GLFWwindow* window, Camera& camera, float deltaTime);
	void UpdateObjectKeyboard(GLFWwindow* window, SceneData& scene, std::size_t selectObjectIndex, float deltaTime);
	void UpdateLightKeyboard(GLFWwindow* window, glm::vec3& lightColor);
	void UpdateExitKeyboard(GLFWwindow* window);
	bool DeleteSelectedObject(SceneData&  scene, std::size_t& selectedObjectIndex);
private:
	float m_ObjectMoveSpeed = 0.8f;
	float m_ObjectRotationSpeed = 90.0f;
};