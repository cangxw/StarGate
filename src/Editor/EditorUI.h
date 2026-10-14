#pragma once
#include <filesystem>
#include "SceneData.h"

//前置声明主要有两个好处：
//- 减少编译依赖。 包含 EditorUI.h 的文件，不必一起包含 Framebuffer.h 以及它引用的其他头文件。大型项目中，这有助于减少编译时间。
//- 减少循环包含。 如果两个类需要互相保存对方的指针或引用，可以通过前置声明避免两个头文件互相包含。
// 声明一个指针 Framebuffer*	足够
// 声明一个引用参数 Framebuffer& 足够
//直接存放一个对象 Framebuffer m_Framebuffer;	不够，需要完整定义
//调用 framebuffer.Bind() 等成员函数	不够，需要完整定义
//使用 sizeof(Framebuffer)	不够，需要完整定义
class Framebuffer;
struct GLFWwindow;

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

struct EditorKeyboardState
{
	bool captureKeyboard = false;
	bool deletePressed = false;
};


class EditorUI
{
public:
	void Initialize(GLFWwindow* window);
	void BeginFrame();
	void Render();
	void Shutdown();

	void DrawDockSpace();
	EditorMenuActions DrawMenuBar();
	void DrawHierarchy(const SceneData& scene, std::size_t& selectedObjectIndex);
	void DrawAssets(const std::filesystem::path& assetRoot, std::filesystem::path& currentAssetDirectory,
		std::string& pendingModelPath, std::string& sceneSaveMessage);
	void DrawInspector(SceneData& scene, std::size_t& selectedObjectIndex, glm::vec3& lightColor,
		float& ambientStrength, const std::string& statusMessage);
	SceneViewState DrawScene(Framebuffer& sceneFramebuffer);
	EditorKeyboardState ReadKeyboardState();
};