#pragma once
#include <string>
#include "ImGuiComponent.h"
#include "layer/EditorLayer.h"

#include "Application.h"
#include "objects/GameObject.h"
#include "imgui/ImGuiConsole.h"
#include "components/AudioComponent.h"

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
					// All attributes of components
					selectedGO->OnImGuiRender();

					// Add Component
					if (ImGui::Button("Add Component")) {
						ImGui::OpenPopup("Add Component Popup");
					}
					if (ImGui::BeginPopup("Add Component Popup"))
					{
						if (ImGui::BeginMenu("Audio")) {
							if (ImGui::MenuItem("Audio Listener")) {
								selectedGO->AddComponent(new AudioListener());
							}
							if (ImGui::MenuItem("Audio Source")) {
								selectedGO->AddComponent(new AudioSource());
							}
							ImGui::EndMenu();
						}
						if (ImGui::BeginMenu("Physics")) {
							if (!selectedGO->GetComponent<PhysicsComponent>()) {
								if (ImGui::MenuItem("Rigidbody")) {
									selectedGO->AddComponent(new Rigidbody());
								}
							}
							else {
								if (ImGui::MenuItem("Collider")) {
									selectedGO->AddComponent(new AABB());
								}
							}
							ImGui::EndMenu();
						}
						ImGui::Separator();
						ImGui::Selectable("Lobster");
						ImGui::EndPopup();
					}
				}

				ImGui::PopStyleVar();
			}
			ImGui::End();
		}
	};
}
