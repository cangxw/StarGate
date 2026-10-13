#include "EditorUI.h"
#include <imgui.h>

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