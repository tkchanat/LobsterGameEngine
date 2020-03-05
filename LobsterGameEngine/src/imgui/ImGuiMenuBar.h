#pragma once
#include "ImGuiComponent.h"
#include "layer/EditorLayer.h"
#include "events/EventQueue.h"
#include "events/EventCollection.h"
#include "imgui/ImGuiAbout.h"
#include "imgui/ImGuiExport.h"
#include "imgui/ImGuiScene.h"
#include "imgui/ImGuiNodeGraphEditor.h"
#include "graphics/meshes/MeshFactory.h"
#include "graphics/Skybox.h"
#include "system/UndoSystem.h"

namespace Lobster
{

	class ImGuiMenuBar : public ImGuiComponent
	{
	private:
		ImGuiAbout* m_about;
		ImGuiNodeGraphEditor* m_nodeGraphEditor;
		ImGuiExport* m_export;
		int m_prevHighlightIndex = -1;	//	Used for undo / redo or other multiple highlight menu items.
		bool b_show_export = false;
		bool b_show_nodeGraphEditor = false;
	public:
		ImGuiMenuBar() : 
			m_about(new ImGuiAbout()), 
			m_export(new ImGuiExport()),
			m_nodeGraphEditor(new ImGuiNodeGraphEditor())
		{
			EventDispatcher::AddCallback(EVENT_KEY_PRESSED, new EventCallback<KeyPressedEvent>([this](KeyPressedEvent* e) {
				if (e->Mod & GLFW_MOD_CONTROL) {
					if (e->Mod & GLFW_MOD_SHIFT) {
						if (e->Key == GLFW_KEY_S) SaveAs();
						if (e->Key == GLFW_KEY_E) b_show_export = true;
						if (e->Key == GLFW_KEY_Z) UndoSystem::GetInstance()->Redo();
					}
					else {
						if (e->Key == GLFW_KEY_N) New();
						if (e->Key == GLFW_KEY_O) Open();
						if (e->Key == GLFW_KEY_S) Save();
						if (e->Key == GLFW_KEY_Z) UndoSystem::GetInstance()->Undo();
					}
				}
			}));
		}
		~ImGuiMenuBar() { 
			if (m_about) delete m_about; 
			if (m_export) delete m_export; 
			m_about = nullptr; 
			m_export = nullptr; 
		}
		virtual void Show(bool* p_open) override
		{
			if (ImGui::BeginMenuBar())
			{
				// ==========================================
				// File
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("New", "Ctrl+N", false)) {
						New();
					}
					if (ImGui::MenuItem("Open", "Ctrl+O", false)) {
						Open();
					}
					if (ImGui::MenuItem("Save", "Ctrl+S", false)) {
						Save();
					}
					if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S", false)) {
						SaveAs();
					}
					if (ImGui::MenuItem("Export...", "Ctrl+Shift+E", false)) {
						b_show_export = true;
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Quit", "Alt+F4", false)) {
						EventQueue::GetInstance()->AddEvent<WindowClosedEvent>();
					}
					ImGui::EndMenu();
				}
				if (b_show_export) m_export->Show(&b_show_export);
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
				// Scene Settings
				static bool show_skyboxEditor = false;
				static bool use_deferred_pipeline = false;
				if (ImGui::BeginMenu("Settings"))
				{
					if (ImGui::BeginMenu("Editor")) {
						ImGuiScene::ShowGridSettings();
						ImGui::EndMenu();
					}
					if (ImGui::MenuItem("Skybox")) {
						show_skyboxEditor = true;
					}
					if (ImGui::BeginMenu("Render pipelines")) {
						ImGui::Checkbox("Use deferred pipeline", &use_deferred_pipeline);
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Standardized to PBR shading");
						if (ImGui::IsItemDeactivatedAfterChange()) {
							Renderer::SetDeferredPipeline(use_deferred_pipeline);
						}
						ImGui::EndMenu();
					}
					if (ImGui::MenuItem("Node Graph")) {
						b_show_nodeGraphEditor = true;
					}
					ImGui::EndMenu();
				}
				if (show_skyboxEditor) {
					GetScene()->GetSkybox()->OnImGuiRender(&show_skyboxEditor);
				}
				if (b_show_nodeGraphEditor) {
					m_nodeGraphEditor->Show(&b_show_nodeGraphEditor);
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

		void New() {
			// Clear current resource cache
			TextureLibrary::Clear();
			// TODO new a scene in a more proper way
			Application* app = Application::GetInstance();
			app->OpenScene("");
			GameObject* camera = new GameObject("Main Camera");
			CameraComponent* comp = new CameraComponent();
			camera->AddComponent(comp);
			camera->AddComponent(new AudioListener());
			camera->transform.Translate(0, 2, 10);
			app->GetCurrentScene()->AddGameObject(camera);
			GameObject* light = new GameObject("Directional Light");
			light->AddComponent(new LightComponent(LightType::DIRECTIONAL_LIGHT));
			light->transform.Translate(0, 2, 3);
			app->GetCurrentScene()->AddGameObject(light);
		}

		void Open() {
			// Clear current resource cache
			TextureLibrary::Clear();
			// Open scene
			Application* app = Application::GetInstance();
			std::string fullpath = FileSystem::OpenFileDialog();
			if (!fullpath.empty()) {
				app->OpenScene(fullpath.c_str());
			}
		}

		void Save() {
			Application* app = Application::GetInstance();
			std::string scenePath = app->GetScenePath();
			if (scenePath.empty()) {
				SaveAs(); // save as
			}
			else {
				// save by overwrite
				std::stringstream ss = GetScene()->Serialize();
				FileSystem::WriteStringStream(scenePath.c_str(), ss);
				Application::GetInstance()->SetSaved(true);
			}
		}

		void SaveAs() {
			Application* app = Application::GetInstance();
			std::string fullpath = FileSystem::SaveFileDialog(".");
			if (!fullpath.empty()) {
				std::stringstream ss = GetScene()->Serialize();
				FileSystem::WriteStringStream(fullpath.c_str(), ss);
				app->SetScenePath(fullpath.c_str());
				app->SetSaved(true);
			}
		}
	};

}
