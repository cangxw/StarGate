#pragma once
#include <filesystem>
#include "SceneData.h"

struct EditorMenuActions
{
	bool saveScene = false;
	bool loadScene = false;
	bool exit = false;
};

class EditorUI
{
public:
	void DrawDockSpace();
	EditorMenuActions DrawMenuBar();
	void DrawHierarchy(const SceneData& scene, std::size_t& selectedObjectIndex);
	void DrawAssets(const std::filesystem::path& assetRoot, std::filesystem::path& currentAssetDirectory,
		std::string& pendingModelPath, std::string& sceneSaveMessage);
	void DrawInspector(SceneData& scene, std::size_t& selectedObjectIndex, glm::vec3& lightColor,
		float& ambientStrength, const std::string& statusMessage);
};