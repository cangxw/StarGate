#include "EditorUI.h"
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>	//glm::value_ptr 用来取得 GLM 向量或矩阵内部数据的指针，方便传给其他函数

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