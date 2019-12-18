#pragma once
#include "ImGuiComponent.h"
#include "layer/EditorLayer.h"
#include "events/EventQueue.h"
#include "events/EventCollection.h"
#include "imgui/ImGuiAbout.h"
#include "graphics/meshes/MeshFactory.h"
#include "system/UndoSystem.h"

namespace Lobster
{
	class ImGuiMenuBar : public ImGuiComponent
	{
	private:
		Scene* scene;
		ImGuiAbout* m_about;
	public:
		ImGuiMenuBar(Scene* scene) : scene(scene), m_about(new ImGuiAbout()) {}
		virtual void Show(bool* p_open) override
		{
			if (ImGui::BeginMenuBar())
			{
				// ==========================================
				// File
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("New", "Ctrl+N", false))
					{
					}
					if (ImGui::MenuItem("Open", "Ctrl+O", false))
					{
					}
					if (ImGui::MenuItem("Save", "Ctrl+S", false))
					{
					}
					if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S", false))
					{
					}
					if (ImGui::MenuItem("Export...", "", false)) {

					}
					ImGui::Separator();
					if (ImGui::MenuItem("Quit", "Alt+F4", false))
					{
						EventQueue::GetInstance()->AddEvent<WindowClosedEvent>();
					}
					ImGui::EndMenu();
				}
				// ==========================================
				// Edit
				if (ImGui::BeginMenu("Edit"))
				{
					UndoSystem* undo = UndoSystem::GetInstance();
					if (ImGui::MenuItem("Undo", "Ctrl+Z", false, undo->UndosRemaining() > 0)) {
						undo->Undo();
					}
					if (ImGui::MenuItem("Redo", "Ctrl+Shift+Z", false, undo->RedosRemaining() > 0)) {
						undo->Redo();
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Cut", "Ctrl+X")) {}
					if (ImGui::MenuItem("Copy", "Ctrl+C")) {}
					if (ImGui::MenuItem("Paste", "Ctrl+V")) {}
					ImGui::EndMenu();
				}
				// ==========================================
				// GameObject
				if (ImGui::BeginMenu("GameObject"))
				{
					if (ImGui::MenuItem("Create Empty", "", false)) {}
					if (ImGui::BeginMenu("Create Primitive"))
					{
						if (ImGui::MenuItem("Cube", "", false)) {
							GameObject* cube = new GameObject("Cube");
							cube->AddComponent<MeshComponent>(MeshFactory::Cube(), glm::vec3(-1, -1, -1), glm::vec3(1, 1, 1));
							scene->AddGameObject(cube);
						}
						if (ImGui::MenuItem("Sphere", "", false)) {
							GameObject* sphere = new GameObject("Sphere");
							sphere->AddComponent<MeshComponent>(MeshFactory::Sphere(1, 32, 32), glm::vec3(-1, -1, -1), glm::vec3(1, 1, 1));
							scene->AddGameObject(sphere);
						}
						if (ImGui::MenuItem("Plane", "", false)) {
							GameObject* plane = new GameObject("Plane");
							plane->AddComponent<MeshComponent>(MeshFactory::Plane(), glm::vec3(-1, -1, 0), glm::vec3(1, 1, 0));
							scene->AddGameObject(plane);
						}
						ImGui::EndMenu();
					}
					ImGui::EndMenu();
				}
				// ==========================================
				// Window
				if (ImGui::BeginMenu("Window"))
				{
					if (ImGui::MenuItem("Reset Layout", "", false))
					{
						EditorLayer::ResetDefaultLayout();
					}
					ImGui::EndMenu();
				}
				// ==========================================
				// Help
				static bool show_about = false;
				if (ImGui::BeginMenu("Help"))
				{
					if (ImGui::MenuItem("About Lobster Engine", "", false))
					{
						show_about = true;
					}
					ImGui::EndMenu();
				}
				if(show_about) m_about->Show(&show_about);
				ImGui::EndMenuBar();
			}
			ImGui::End();
		}
	};

}
