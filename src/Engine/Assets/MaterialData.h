#pragma once

#include <glm/glm.hpp>
#include <string>

struct MaterialData
{
    // 漫反射颜色，默认白色
    glm::vec3 diffuseColor{ 1.0f, 1.0f, 1.0f };

    // 漫反射贴图路径，空字符串表示没有这张贴图
    std::string diffuseTexturePath;
};