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
		ImGuiAbout* m_about;
		int m_prevHighlightIndex = -1;	//	Used for undo / redo or other multile highlight menu items.

	public:
		ImGuiMenuBar() : m_about(new ImGuiAbout()) {}
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
						std::string path = FileSystem::OpenFileDialog();
						if (!path.empty()) {
							Application::GetInstance()->OpenScene(path.c_str());
						}
					}
					if (ImGui::MenuItem("Save", "Ctrl+S", false))
					{
						std::string scenePath = Application::GetInstance()->GetScenePath();
						if (scenePath.empty()) {
							goto LABEL_SAVEAS; // save as
						}
						else {
							// save by overwrite
							std::stringstream ss = GetScene()->Serialize();
							FileSystem::WriteStringStream(FileSystem::Path(scenePath).c_str(), ss);
							Application::GetInstance()->SetSaved(true);
						}								
					}
					if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S", false))
					{
					LABEL_SAVEAS:
						std::string fullpath = FileSystem::SaveFileDialog(".");
						if (!fullpath.empty()) {
							std::stringstream ss = GetScene()->Serialize();
							FileSystem::WriteStringStream(fullpath.c_str(), ss);
							Application::GetInstance()->SetScenePath(fullpath.c_str());
							Application::GetInstance()->SetSaved(true);
						}						
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
					if (undo->UndosRemaining() > 1) {
						if (ImGui::BeginMenu("Undo multiple")) {
							int i = 0;
							bool noneSelected = true;
							for (std::string& item : undo->UndoList()) {
								if (ImGui::MenuItem(item.c_str(), "", i <= m_prevHighlightIndex)) {
									undo->Undo(i + 1);
								}
								if (ImGui::IsItemHovered()) {
									m_prevHighlightIndex = i;
									noneSelected = false;
								}
								i++;
							}
							ImGui::Separator();
							if (ImGui::MenuItem("Undo all")) {
								undo->Undo(undo->UndosRemaining());
							}
							if (ImGui::IsItemHovered()) {
								m_prevHighlightIndex = i;
								noneSelected = false;
							}
							ImGui::EndMenu();

							if (noneSelected) m_prevHighlightIndex = -1;
						}
					}

					if (ImGui::MenuItem("Redo", "Ctrl+Shift+Z", false, undo->RedosRemaining() > 0)) {
						undo->Redo();
					}
					if (undo->RedosRemaining() > 1) {
						if (ImGui::BeginMenu("Redo multiple")) {
							int i = 0;
							bool noneSelected = true;
							for (std::string& item : undo->RedoList()) {
								if (ImGui::MenuItem(item.c_str(), "", i <= m_prevHighlightIndex)) {
									undo->Redo(i + 1);
								}
								if (ImGui::IsItemHovered()) {
									m_prevHighlightIndex = i;
									noneSelected = false;
								}
								i++;
							}
							ImGui::Separator();
							if (ImGui::MenuItem("Redo all")) {
								undo->Redo(undo->RedosRemaining());
							}
							if (ImGui::IsItemHovered()) {
								m_prevHighlightIndex = i;
								noneSelected = false;
							}
							ImGui::EndMenu();

							if (noneSelected) m_prevHighlightIndex = -1;
						}
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
					Scene* scene = GetScene();
					if (ImGui::MenuItem("Create Empty", "", false)) {
						GameObject* empty = new GameObject("Empty");
						scene->AddGameObject(empty);
						UndoSystem::GetInstance()->Push(new CreateObjectCommand(empty, scene));
					}
					if (ImGui::BeginMenu("Create Primitive"))
					{						
						if (ImGui::MenuItem("Cube", "", false)) {
							GameObject* cube = new GameObject("Cube");
							cube->AddComponent(new MeshComponent(PrimitiveShape::CUBE));
							scene->AddGameObject(cube);
							UndoSystem::GetInstance()->Push(new CreateObjectCommand(cube, scene));
						}
						if (ImGui::MenuItem("Sphere", "", false)) {
							GameObject* sphere = new GameObject("Sphere");
							sphere->AddComponent(new MeshComponent(PrimitiveShape::SPHERE));
							scene->AddGameObject(sphere);
							UndoSystem::GetInstance()->Push(new CreateObjectCommand(sphere, scene));
						}
						if (ImGui::MenuItem("Plane", "", false)) {
							GameObject* plane = new GameObject("Plane");
							plane->AddComponent(new MeshComponent(PrimitiveShape::PLANE));
							scene->AddGameObject(plane);
							UndoSystem::GetInstance()->Push(new CreateObjectCommand(plane, scene));
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
		}
	};

}
