#pragma once
#include <string>
#include "ImGuiComponent.h"
#include "layer/EditorLayer.h"

#include "Application.h"
#include "objects/GameObject.h"
#include "imgui/ImGuiConsole.h"
#include "system/UndoSystem.h"

namespace Lobster
{

	class ImGuiToolbar : public ImGuiComponent
	{
	private:
		Scene* scene;
		const static int numBtn = 7;		
		std::string image_path[numBtn] = { 
			"textures/ui/undo.png", "textures/ui/undo_grey.png",
			"textures/ui/redo.png", "textures/ui/redo_grey.png",
			"textures/ui/plane.png", 
			"textures/ui/cube.png", 
			"textures/ui/sphere.png" };
		Texture2D* m_tex[numBtn];
		static GameObject* selectedObj;
	public:
		ImGuiToolbar(Scene* scene) : ImGuiComponent(), scene(scene) {
			for (int i = 0; i < numBtn; i++) {
				// load image (texture)
				m_tex[i] = TextureLibrary::Use(FileSystem::Path(image_path[i]).c_str());
			}
		}

		virtual void Show(bool* p_open) override
		{
			ImGui::SetNextWindowSize(ImVec2(0, 28));
			if (!ImGui::Begin("Toolbar", p_open, ImGuiWindowFlags_NoResize))
			{
				ImGui::End();
				return;
			}
			int frame_padding = 2;
			UndoSystem* undo = UndoSystem::GetInstance();
			// Undo
			if (undo->UndosRemaining()) {				
				if (ImGui::ImageButton(m_tex[0]->Get(), ImVec2(24, 24), ImVec2(0, 0), ImVec2(1, 1), frame_padding, ImColor(0, 0, 0, 255))) {
					undo->Undo();
				}
			}
			else {
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
				ImGui::ImageButton(m_tex[1]->Get(), ImVec2(24, 24), ImVec2(0, 0), ImVec2(1, 1), frame_padding, ImColor(0, 0, 0, 255));
				ImGui::PopStyleVar();
				ImGui::PopItemFlag();
			}
			ImGui::SameLine();
			// Redo
			if (undo->RedosRemaining()) {
				if (ImGui::ImageButton(m_tex[2]->Get(), ImVec2(24, 24), ImVec2(0, 0), ImVec2(1, 1), frame_padding, ImColor(0, 0, 0, 255))) {
					undo->Redo();
				}
			}
			else {
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
				ImGui::ImageButton(m_tex[3]->Get(), ImVec2(24, 24), ImVec2(0, 0), ImVec2(1, 1), frame_padding, ImColor(0, 0, 0, 255));
				ImGui::PopStyleVar();
				ImGui::PopItemFlag();
			}
			ImGui::SameLine();
			// Plane Generation
			if (ImGui::ImageButton(m_tex[4]->Get(), ImVec2(24, 24), ImVec2(0, 0), ImVec2(1, 1), frame_padding, ImColor(0, 0, 0, 255))) {					
				GameObject* plane = new GameObject("Plane");
				plane->AddComponent(new MeshComponent(MeshFactory::Plane(), glm::vec3(-1, -1, 0), glm::vec3(1, 1, 0)));
				scene->AddGameObject(plane);
			}
			ImGui::SameLine();
			// Cube Generation
			if (ImGui::ImageButton(m_tex[5]->Get(), ImVec2(24, 24), ImVec2(0, 0), ImVec2(1, 1), frame_padding, ImColor(0, 0, 0, 255))) {
				GameObject* cube = new GameObject("Cube");
				cube->AddComponent(new MeshComponent(MeshFactory::Cube(), glm::vec3(-1, -1, -1), glm::vec3(1, 1, 1)));
				scene->AddGameObject(cube);
			}
			ImGui::SameLine();
			// Sphere Generation
			if (ImGui::ImageButton(m_tex[6]->Get(), ImVec2(24, 24), ImVec2(0, 0), ImVec2(1, 1), frame_padding, ImColor(0, 0, 0, 255))) {
				GameObject* sphere = new GameObject("Sphere");
				sphere->AddComponent(new MeshComponent(MeshFactory::Sphere(1, 32, 32)));
				scene->AddGameObject(sphere);
			}			
			ImGui::End();
		}
	};
}
