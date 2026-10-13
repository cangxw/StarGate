#include "EditorUI.h"
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>	//glm::value_ptr 用来取得 GLM 向量或矩阵内部数据的指针，方便传给其他函数
#include <imgui_internal.h>

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

void EditorUI::DrawInspector(SceneData& scene, std::size_t& selectedObjectIndex, glm::vec3& lightColor,
	float& ambientStrength, const std::string& statusMessage)
{
	if (ImGui::Begin("Inspector"))
	{
		if (selectedObjectIndex < scene.objects.size())
		{
			SceneObject& selectedObject = scene.objects[selectedObjectIndex];
			ImGui::Text( "Selected: %s", selectedObject.name.c_str());

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