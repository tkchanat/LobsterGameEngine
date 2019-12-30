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
							PhysicsComponent* physics = selectedGO->GetComponent<PhysicsComponent>();
							if (!physics || !physics->IsEnabled()) {
								if (ImGui::Selectable("Enable Rigidbody")) {
									physics->SetEnabled(true);
								}
							}
							else {
								if (ImGui::Selectable("Add Collider")) {
									AABB* aabb = new AABB(physics);
									aabb->SetOwner(selectedGO);
									aabb->SetOwnerTransform(&selectedGO->transform);
									physics->AddCollider(aabb);
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
