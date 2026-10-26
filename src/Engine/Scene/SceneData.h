#pragma once

#include <string>
#include <vector>
#include "SceneObject.h"

struct SceneData
{
    std::string modelPath;
    std::vector<SceneObject> objects;
};