#pragma once
#include <string>
#include "ImGuiComponent.h"
#include "layer/EditorLayer.h"

#include "Application.h"
#include "objects/GameObject.h"
#include "imgui/ImGuiConsole.h"
#include "imgui/ImGuiGame.h"
#include "system/UndoSystem.h"

namespace Lobster
{

	class ImGuiToolbar : public ImGuiComponent
	{
	private:
		const float spacer_width = 10.f;
		const static int numIco = 14;		
		std::string image_path[numIco] = { 
			"textures/ui/undo.png", "textures/ui/undo_grey.png", "textures/ui/redo.png", "textures/ui/redo_grey.png",
			"textures/ui/plane.png", "textures/ui/cube.png", "textures/ui/sphere.png", "textures/ui/particles.png",
			"textures/ui/pt_light.png", "textures/ui/dir_light.png", "textures/ui/simulate.png", "textures/ui/simuend.png",
			"textures/ui/play.png", "textures/ui/stop.png" };
		static GameObject* selectedObj;
		ImGuiGame* m_gameView = nullptr;
		bool m_showGameView = true;
	public:
		ImGuiToolbar() {}

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
			// Undo =============
			if (undo->UndosRemaining()) {
				Texture2D* tex = TextureLibrary::Use(image_path[0].c_str());
				if (ImGui::ImageButton(tex->Get(), ImVec2(24, 24), ImVec2(0, 0), ImVec2(1, 1), frame_padding, ImColor(0, 0, 0, 255))) {
					undo->Undo();
				}
			}
			else {
				Texture2D* tex = TextureLibrary::Use(image_path[1].c_str());
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
				ImGui::ImageButton(tex->Get(), ImVec2(24, 24), ImVec2(0, 0), ImVec2(1, 1), frame_padding, ImColor(0, 0, 0, 255));
				ImGui::PopStyleVar();
				ImGui::PopItemFlag();
			}
			ImGui::SameLine();
			// Redo =============
			if (undo->RedosRemaining()) {
				Texture2D* tex = TextureLibrary::Use(image_path[2].c_str());
				if (ImGui::ImageButton(tex->Get(), ImVec2(24, 24), ImVec2(0, 0), ImVec2(1, 1), frame_padding, ImColor(0, 0, 0, 255))) {
					undo->Redo();
				}
			}
			else {
				Texture2D* tex = TextureLibrary::Use(image_path[3].c_str());
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
				ImGui::ImageButton(tex->Get(), ImVec2(24, 24), ImVec2(0, 0), ImVec2(1, 1), frame_padding, ImColor(0, 0, 0, 255));
				ImGui::PopStyleVar();
				ImGui::PopItemFlag();
			}
			ImGui::SameLine();
			ImGui::Dummy(ImVec2(spacer_width, 0.0f));
			ImGui::SameLine();
			Scene* scene = GetScene();
			// Plane Generation =============
			{
				Texture2D* tex = TextureLibrary::Use(image_path[4].c_str());
				if (ImGui::ImageButton(tex->Get(), ImVec2(24, 24), ImVec2(0, 0), ImVec2(1, 1), frame_padding, ImColor(0, 0, 0, 255))) {
					GameObject* plane = new GameObject("Plane");
					plane->AddComponent(new MeshComponent(PrimitiveShape::PLANE));
					scene->AddGameObject(plane);
					UndoSystem::GetInstance()->Push(new CreateObjectCommand(plane, scene));
				}
				ImGui::SameLine();
			}
			// Cube Generation =============
			{
				Texture2D* tex = TextureLibrary::Use(image_path[5].c_str());
				if (ImGui::ImageButton(tex->Get(), ImVec2(24, 24), ImVec2(0, 0), ImVec2(1, 1), frame_padding, ImColor(0, 0, 0, 255))) {
					GameObject* cube = new GameObject("Cube");
					cube->AddComponent(new MeshComponent(PrimitiveShape::CUBE));
					scene->AddGameObject(cube);
					UndoSystem::GetInstance()->Push(new CreateObjectCommand(cube, scene));
				}
				ImGui::SameLine();
			}
			// Sphere Generation =============
			{
				Texture2D* tex = TextureLibrary::Use(image_path[6].c_str());
				if (ImGui::ImageButton(tex->Get(), ImVec2(24, 24), ImVec2(0, 0), ImVec2(1, 1), frame_padding, ImColor(0, 0, 0, 255))) {
					GameObject* sphere = new GameObject("Sphere");
					sphere->AddComponent(new MeshComponent(PrimitiveShape::SPHERE));
					scene->AddGameObject(sphere);
					UndoSystem::GetInstance()->Push(new CreateObjectCommand(sphere, scene));
				}
				ImGui::SameLine();
				ImGui::Dummy(ImVec2(spacer_width, 0.0f));
				ImGui::SameLine();
			}
			// Particle System =============
			{
				Texture2D* tex = TextureLibrary::Use(image_path[7].c_str());
				if (ImGui::ImageButton(tex->Get(), ImVec2(24, 24), ImVec2(0, 0), ImVec2(1, 1), frame_padding, ImColor(0, 0, 0, 255))) {
					GameObject* ps = new GameObject("Particle System");
					ps->AddComponent(new ParticleComponent());
					scene->AddGameObject(ps);
					UndoSystem::GetInstance()->Push(new CreateObjectCommand(ps, scene));
				}
				ImGui::SameLine();
				ImGui::Dummy(ImVec2(spacer_width, 0.0f));
				ImGui::SameLine();
			}
			// Point Light =============
			{
				Texture2D* tex = TextureLibrary::Use(image_path[8].c_str());
				if (ImGui::ImageButton(tex->Get(), ImVec2(24, 24), ImVec2(0, 0), ImVec2(1, 1), frame_padding, ImColor(0, 0, 0, 255))) {
					GameObject* light = new GameObject("Point Light");
					light->AddComponent(new LightComponent(LightType::POINT_LIGHT));
					scene->AddGameObject(light);
					UndoSystem::GetInstance()->Push(new CreateObjectCommand(light, scene));
				}
				ImGui::SameLine();
			}
			// Directional Light =============
			{
				Texture2D* tex = TextureLibrary::Use(image_path[9].c_str());
				if (ImGui::ImageButton(tex->Get(), ImVec2(24, 24), ImVec2(0, 0), ImVec2(1, 1), frame_padding, ImColor(0, 0, 0, 255))) {
					GameObject* light = new GameObject("Directional Light");
					light->AddComponent(new LightComponent(LightType::DIRECTIONAL_LIGHT));
					scene->AddGameObject(light);
					UndoSystem::GetInstance()->Push(new CreateObjectCommand(light, scene));
				}
				ImGui::SameLine();
				ImGui::Dummy(ImVec2(spacer_width, 0.0f));
				ImGui::SameLine();
			}
			// Simulate =============
			if (Application::GetMode() != SIMULATION) {
				Texture2D* tex = TextureLibrary::Use(image_path[10].c_str());
				if (ImGui::ImageButton(tex->Get(), ImVec2(24, 24), ImVec2(0, 0), ImVec2(1, 1), frame_padding, ImColor(0, 0, 0, 255))) {
					Application::SwitchMode(SIMULATION);
					// TODO save scene TODO
					// ...

					// initialize all gameobjects and components
					for (GameObject* gameObject : scene->m_gameObjects) {
						gameObject->OnBegin();
					}
				}
			}
			else {
				Texture2D* tex = TextureLibrary::Use(image_path[11].c_str());
				if (ImGui::ImageButton(tex->Get(), ImVec2(24, 24), ImVec2(0, 0), ImVec2(1, 1), frame_padding, ImColor(0, 0, 0, 255))) {
					Application::SwitchMode(EDITOR);
				}
			}
			ImGui::SameLine();
			// Play =============
			if (Application::GetMode() != GAME) {
				Texture2D* tex = TextureLibrary::Use(image_path[12].c_str());
				if (ImGui::ImageButton(tex->Get(), ImVec2(24, 24), ImVec2(0, 0), ImVec2(1, 1), frame_padding, ImColor(0, 0, 0, 255))) {
					Application::SwitchMode(GAME);
					// TODO save scene TODO
					// ...

					m_gameView = new ImGuiGame();
					m_showGameView = true;
					// initialize all gameobjects and components
					for (GameObject* gameObject : scene->m_gameObjects) {
						gameObject->OnBegin(); // this should be in actual game, not here
					}
				}
			}			
			else {
				// either press stop button or close the tab
				Texture2D* tex = TextureLibrary::Use(image_path[13].c_str());
				if (!m_showGameView || ImGui::ImageButton(tex->Get(), ImVec2(24, 24), ImVec2(0, 0), ImVec2(1, 1), frame_padding, ImColor(0, 0, 0, 255))) {
					Application::SwitchMode(EDITOR);
					// clearup of all gameobjects and components
					for (GameObject* gameObject : scene->m_gameObjects) {
						gameObject->OnEnd();
					}					
					// reinitialize the game view
					if (m_gameView) {
						delete m_gameView;
						m_gameView = nullptr;
					}
					// load the original scene setup
					// Application::GetInstance()->OpenScene("scenes/test.lobster");
				}
			}			
			// show the game mode tab
			if (m_gameView) {
				m_gameView->Show(&m_showGameView);
			}
			ImGui::End();
		}
	};
}
