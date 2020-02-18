#pragma once
#include "ImGuiComponent.h"
#include "graphics/Scene.h"
#include "graphics/Renderer.h"
#include "components/CameraComponent.h"

namespace Lobster {

	class ImGuiGame : public ImGuiComponent {
	private:
		// window-related variable, will be updated per frame
		static ImVec2 window_pos;
		static ImVec2 window_size;
		// profiler
		bool b_showProfiler;
	public:
		ImGuiGame() = default;
		inline static glm::vec2 GetWindowPos() { return glm::vec2(window_pos.x, window_pos.y); }
		inline static glm::vec2 GetWindowSize() { return glm::vec2(window_size.x, window_size.y); }

		virtual void Show(bool* p_open) override {
			// Scene window
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::Begin("Game", p_open, ImGuiWindowFlags_None);
			window_pos = ImGui::GetWindowPos();
			window_size = ImGui::GetWindowSize();
			Config& config = Application::GetInstance()->GetConfig();
			config.gameTabPos = glm::vec2(window_pos.x, window_pos.y);
			config.gameTabSize = glm::vec2(window_size.x, window_size.y);

			// draw scene
			CameraComponent* camera = CameraComponent::GetActiveCamera();
			void* image = camera->GetFrameBuffer()->Get();
			float aspectRatio = camera->GetFrameBuffer()->GetAspectRatio();
			float imageHeight = window_size.x / aspectRatio;
			float startingHeight = window_pos.y + window_size.y / 2.f - imageHeight / 2.f;
			ImVec2 imageMin = ImVec2(window_pos.x, startingHeight);
			ImVec2 imageMax = ImVec2(window_pos.x + window_size.x, startingHeight + imageHeight);
			ImGui::GetWindowDrawList()->AddImage(image, imageMin, imageMax, ImVec2(0, 1), ImVec2(1, 0));

			// TODO user-defined actions

			// frame update
			ImGui::PopStyleVar();
			ImGui::End();
		}

	};

	// static initialization
	ImVec2 ImGuiGame::window_pos = ImVec2(0, 0);
	ImVec2 ImGuiGame::window_size = ImVec2(0, 0);
}