#pragma once

#include "SceneData.h"

class EditorUI
{
public:
	void DrawDockSpace();
	void DrawHierarchy(const SceneData& scene, std::size_t& selectedObjectIndex);
	void DrawInspector(SceneData& scene, std::size_t& selectedObjectIndex, glm::vec3& lightColor,
		float& ambientStrength, const std::string& statusMessage);
};