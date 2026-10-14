#pragma once

#include <cstddef>
#include "SceneObject.h"
#include "SceneData.h"

bool SaveScene(const char* filePath, const char* modelPath, const SceneObject* objects, std::size_t objectCount);

bool LoadScene(const char* filePath, SceneData& scene);