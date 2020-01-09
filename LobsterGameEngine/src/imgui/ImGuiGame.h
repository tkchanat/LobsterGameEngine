#pragma once
#include "ImGuiComponent.h"
#include "graphics/Scene.h"
#include "graphics/Renderer.h"
#include "components/CameraComponent.h"


namespace Lobster {

	class ImGuiGame : public ImGuiComponent {
	private:
		// window-related variable
		ImVec2 window_pos;
		ImVec2 window_size;
		// profiler
		bool b_showProfiler;
	public:
		ImGuiGame() = default;

		virtual void Show(bool* p_open) override {
			// Scene window
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::Begin("Game", p_open, ImGuiWindowFlags_None);
			window_pos = ImGui::GetWindowPos();
			window_size = ImGui::GetWindowSize();

			// draw scene
			CameraComponent* camera = CameraComponent::GetActiveCamera();
			camera->ResizeProjection(window_size.x / window_size.y);
			void* image = camera->GetFrameBuffer()->Get();
			ImGui::GetWindowDrawList()->AddImage(image, ImVec2(window_pos.x, window_pos.y), ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y), ImVec2(0, 1), ImVec2(1, 0));

			// TODO user-defined actions

			// frame update
			ImGui::PopStyleVar();
			ImGui::End();
		}

	};
}