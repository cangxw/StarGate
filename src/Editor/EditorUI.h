#pragma once
#include <filesystem>
#include "SceneData.h"

class Framebuffer;

struct SceneViewState
{
	//如果面板折叠或没有显示空间，两个值会保持 false。
	bool hovered = false;
	bool imageVisible = false;
};

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
	SceneViewState DrawScene(Framebuffer& sceneFramebuffer);
};