#pragma once

#include "SceneData.h"

class EditorUI
{
public:
	void DrawHierarchy(const SceneData& scene, std::size_t& selectedObjectIndex);
};