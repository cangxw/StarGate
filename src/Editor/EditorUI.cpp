#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>	//glm::value_ptr 用来取得 GLM 向量或矩阵内部数据的指针，方便传给其他函数
#include <imgui_internal.h>
#include <filesystem>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "EditorUI.h"
#include "Engine/PathUtils.h"
#include "Framebuffer.h"

static bool IsModelFile(const std::filesystem::path& path)
{
	std::string extension = PathToUtf8(path.extension());

	// 把 extension 中的字符转成小写，并写回原字符串。
	// transform 遍历一段数据，对每个元素执行指定操作，把结果保存到指定位置。
	std::transform(
		extension.begin(),		// 从字符串开头开始
		extension.end(),			// 到字符串末尾结束，不包含 end 本身
		extension.begin(),		// 结果从原字符串开头写入
		[](unsigned char character)
		{
			return static_cast<char>(std::tolower(character));
		}
	);

	return extension == ".fbx" ||
		extension == ".obj" ||
		extension == ".gltf" ||
		extension == ".glb";
}

void EditorUI::Initialize(GLFWwindow* window)
{
	// 创建Imgui上下文
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// 开启面板停靠
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	//使用深色的界面
	ImGui::StyleColorsDark();

	//DPI的适配
	// 获取窗体所在屏幕的显示缩放比例
	const float uiScale = ImGui_ImplGlfw_GetContentScaleForWindow(window);

	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(uiScale);	// 空间间距，边距等尺寸
	style.FontScaleDpi = uiScale;	// 字体大小

	//连接GLFW和OPENGL后端
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");
}

void EditorUI::BeginFrame()
{
	// 开始新的一帧
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void EditorUI::Render()
{
	// IMGUI界面绘制顺序
	ImGui::Render();

	//绘制在3维场景上方
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void EditorUI::Shutdown()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void EditorUI::DrawDockSpace()
{
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	const ImGuiID dockspaceID = ImGui::GetID("StarGateDockSpaceV3");

	if (ImGui::DockBuilderGetNode(dockspaceID) == nullptr)
	{
		ImGuiID centerID = dockspaceID;
		ImGuiID leftID = 0;
		ImGuiID rightID = 0;
		ImGuiID hierarchyID = 0;
		ImGuiID assetsID = 0;

		ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspaceID, viewport->WorkSize);

		// 从整个区域切出左侧 12%
		ImGui::DockBuilderSplitNode(centerID, ImGuiDir_Left, 0.12f, &leftID, &centerID);

		// 从剩余区域切出右侧 23%，约占整个区域的 20%
		ImGui::DockBuilderSplitNode(centerID, ImGuiDir_Right, 0.23f, &rightID, &centerID);

		// 将左侧区域上下各分一半
		ImGui::DockBuilderSplitNode(leftID, ImGuiDir_Up, 0.5f, &hierarchyID, &assetsID);

		// 将面板放入对应区域，名称必须与 Begin() 中一致
		ImGui::DockBuilderDockWindow("Hierarchy", hierarchyID);
		ImGui::DockBuilderDockWindow("Assets", assetsID);
		ImGui::DockBuilderDockWindow("Inspector", rightID);
		ImGui::DockBuilderDockWindow("Scene", centerID);

		ImGui::DockBuilderFinish(dockspaceID);
	}

	// 每帧提交停靠区域
	ImGui::DockSpaceOverViewport(dockspaceID, viewport);
}

EditorMenuActions EditorUI::DrawMenuBar()
{
	EditorMenuActions actions;

	// 顶层菜单
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Save Scene"))
			{
				actions.saveScene = true;
			}

			if (ImGui::MenuItem("Load Scene"))
			{
				actions.loadScene = true;
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Exit"))
			{
				actions.exit = true;
			}

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	return actions;
}

void EditorUI::DrawHierarchy(const SceneData& scene, std::size_t& selectedObjectIndex)
{
	if (ImGui::Begin("Hierarchy"))
	{
		for (std::size_t i = 0; i < scene.objects.size(); ++i)
		{
			//后面的控件生成 ID 时包含,它需要成对使用，否则这个索引会继续影响后面的控件。
			ImGui::PushID(static_cast<int>(i));

			const bool selected = (i == selectedObjectIndex);

			if (ImGui::Selectable(scene.objects[i].name.c_str(), selected))
			{
				selectedObjectIndex = i;
			}

			ImGui::PopID();
		}
	}

	ImGui::End();
}

void EditorUI::DrawAssets(const std::filesystem::path& assetRoot, std::filesystem::path& currentAssetDirectory,
	std::string& pendingModelPath, std::string& sceneSaveMessage)
{
	if (ImGui::Begin("Assets"))
	{
		// 根目录时禁止继续向上返回
		ImGui::BeginDisabled(currentAssetDirectory == assetRoot);

		if (ImGui::Button("Up"))
		{
			currentAssetDirectory = currentAssetDirectory.parent_path();
		}

		ImGui::EndDisabled();

		const std::filesystem::path relativePath = currentAssetDirectory.lexically_relative(assetRoot);
		const std::string directoryLabel = currentAssetDirectory == assetRoot
			? "assets"
			: "assets/" + PathToUtf8(relativePath);

		ImGui::TextWrapped("%s", directoryLabel.c_str());
		ImGui::Separator();

		std::error_code error;
		std::filesystem::directory_iterator iterator(currentAssetDirectory, error);
		const std::filesystem::directory_iterator end;
		for (; !error && iterator != end; iterator.increment(error))
		{
			const std::filesystem::directory_entry& entry = *iterator;
			const bool isDirectory = entry.is_directory(error);

			if (error)
			{
				break;
			}

			const std::string filename = PathToUtf8(entry.path().filename());
			const std::string label =
				isDirectory ? "[Dir] " + filename : filename;

			const std::string itemID = PathToUtf8(entry.path());
			ImGui::PushID(itemID.c_str());

			const bool clicked = ImGui::Selectable(
				label.c_str(),
				false,
				ImGuiSelectableFlags_AllowDoubleClick
			);

			const bool doubleClicked =
				clicked &&
				ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

			ImGui::PopID();

			if (doubleClicked)
			{
				if (isDirectory)
				{
					currentAssetDirectory = entry.path();
					break;
				}

				if (IsModelFile(entry.path()))
				{
					pendingModelPath = PathToUtf8(
						entry.path().lexically_relative(assetRoot)
					);

					sceneSaveMessage = "Loading model...";
					break;
				}
			}
		}

		if (error)
		{
			ImGui::TextWrapped("Cannot read directory: %s",
				error.message().c_str());
		}
	}
	ImGui::End();
}

void EditorUI::DrawInspector(SceneData& scene, std::size_t& selectedObjectIndex, glm::vec3& lightColor,
	float& ambientStrength, const std::string& statusMessage)
{
	if (ImGui::Begin("Inspector"))
	{
		if (selectedObjectIndex < scene.objects.size())
		{
			SceneObject& selectedObject = scene.objects[selectedObjectIndex];
			ImGui::Text("Selected: %s", selectedObject.name.c_str());

			ImGui::DragFloat3("Position", glm::value_ptr(selectedObject.position), 0.01f);
			ImGui::DragFloat3("Rotation", glm::value_ptr(selectedObject.rotation), 0.5f);
			ImGui::DragFloat3("Scale", glm::value_ptr(selectedObject.scale), 0.01f, 0.1f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

			if (ImGui::Button("Reset Transform"))
			{
				selectedObject.position = glm::vec3(0.0f);
				selectedObject.rotation = glm::vec3(0.0f);
				selectedObject.scale = glm::vec3(1.0f);
			}
		}
		else
		{
			ImGui::TextUnformatted("No object selected.");
		}

		ImGui::Separator();

		if (!statusMessage.empty())
		{
			ImGui::TextWrapped("%s", statusMessage.c_str());
		}

		ImGui::Separator();
		ImGui::TextUnformatted("Lighting");

		ImGui::ColorEdit3("Light Color", glm::value_ptr(lightColor));
		ImGui::SliderFloat("Ambient", &ambientStrength, 0.0f, 1.0f);

		ImGui::Separator();
		ImGui::Text("FPS:%.1f", ImGui::GetIO().Framerate);
	}

	ImGui::End();
}

SceneViewState EditorUI::DrawScene(Framebuffer& sceneFramebuffer)
{
	SceneViewState state;

	if (ImGui::Begin("Scene", nullptr,
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse))
	{
		const ImGuiIO& io = ImGui::GetIO();
		// 面板中的剩余可用空间
		const ImVec2 sceneSize = ImGui::GetContentRegionAvail();

		// 将界面尺寸转换成实际图像的像素尺寸
		const int sceneWidth = static_cast<int>(sceneSize.x * io.DisplayFramebufferScale.x);
		const int sceneHeight = static_cast<int>(sceneSize.y * io.DisplayFramebufferScale.y);

		if (sceneWidth > 0 && sceneHeight > 0)
		{
			sceneFramebuffer.Resize(sceneWidth, sceneHeight);

			const ImTextureRef sceneTexture(
				static_cast<ImTextureID>(sceneFramebuffer.GetColorTexture())
			);

			ImGui::Image(sceneTexture, sceneSize, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

			state.hovered = ImGui::IsItemHovered();
			state.imageVisible = true;	//如果 Scene 面板折叠了，或者内部没有足够的显示空间，这段代码不会执行，变量就保持 false。
		}
	}
	ImGui::End();

	return state;
}

EditorKeyboardState EditorUI::ReadKeyboardState()
{
	EditorKeyboardState state;

	state.captureKeyboard = ImGui::GetIO().WantCaptureKeyboard;
	state.deletePressed = ImGui::IsKeyPressed(ImGuiKey_Delete, false);

	return state;
}