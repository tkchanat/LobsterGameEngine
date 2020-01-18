#pragma once
#include <string>
#include "ImGuiComponent.h"
#include "layer/EditorLayer.h"

#include "Application.h"
#include "objects/GameObject.h"
#include "imgui/ImGuiConsole.h"
#include "components/AudioComponent.h"
#include "system/UndoSystem.h"
#include "scripts/Script.h"
#include "components/ParticleComponent.h"

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

					// align the "Add Component" button at center horizontally
					ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.1);
					float sizeX = ImGui::GetWindowSize().x * 0.8;
					if (ImGui::Button("Add Component", ImVec2(sizeX, 24))) {
						ImGui::OpenPopup("Add Component Popup");
					}
					if (ImGui::BeginPopup("Add Component Popup"))
					{
						if (ImGui::BeginMenu("Audio")) {
							if (ImGui::MenuItem("Audio Listener")) {
								AudioListener* audioListener = new AudioListener();
								selectedGO->AddComponent(audioListener);
								UndoSystem::GetInstance()->Push(new CreateComponentCommand(audioListener, selectedGO));
							}
							if (ImGui::MenuItem("Audio Source")) {
								AudioSource* audioSource = new AudioSource();
								selectedGO->AddComponent(audioSource);
								UndoSystem::GetInstance()->Push(new CreateComponentCommand(audioSource, selectedGO));
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
									BoxCollider* box = new BoxCollider(physics);
									box->SetOwner(selectedGO);
									box->SetOwnerTransform(&selectedGO->transform);
									physics->AddCollider(box);
								}
							}
							ImGui::EndMenu();
						}
						if (ImGui::Selectable("Particle System")) {
							ParticleComponent* particleComp = new ParticleComponent();
							selectedGO->AddComponent(particleComp);
							UndoSystem::GetInstance()->Push(new CreateComponentCommand(particleComp, selectedGO));
						}
						if (ImGui::Selectable("Script")) {
							selectedGO->AddComponent(new Script());
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
