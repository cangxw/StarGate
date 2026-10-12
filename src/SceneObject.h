#pragma once

#include <string>
#include <glm/glm.hpp>

struct SceneObject
{
	std::string name = "Object";

	glm::vec3 position{ 0.0f, 0.0f, 0.0f };

	glm::vec3 rotation{ 0.0f, 0.0f, 0.0f };

	glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
};