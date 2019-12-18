#pragma once
#include <string>
#include "ImGuiComponent.h"
#include "layer/EditorLayer.h"

#include "Application.h"
#include "objects/GameObject.h"
#include "imgui/ImGuiConsole.h"

namespace Lobster
{

	class ImGuiProperties : public ImGuiComponent
	{
	public:
		virtual void Show(bool* p_open) override
		{
			if (ImGui::Begin("Properties", nullptr))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));

				GameObject* selectedGO = EditorLayer::s_selectedGameObject;
				if (selectedGO)
				{
					selectedGO->OnImGuiRender();
				}

				ImGui::PopStyleVar();
			}
			ImGui::End();
		}
	};
}
