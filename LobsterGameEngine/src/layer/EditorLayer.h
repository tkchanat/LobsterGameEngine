#pragma once
#include "Layer.h"

namespace Lobster
{

	class Scene;
	class Renderer;
	class CameraComponent;
	class ImGuiMenuBar;
	class ImGuiScene;
	class ImGuiDemoWindow;
	class ImGuiComponent;

	class EditorLayer : public Layer
	{
	private:
		static uint s_dockspace_id;
		ImGuiMenuBar* m_menuBar;
		ImGuiScene* m_scene;
		ImGuiComponent* m_toolbar;
		ImGuiComponent* m_properties;
		ImGuiComponent* m_console;
		ImGuiComponent* m_assets;
		ImGuiDemoWindow* m_demoWindow;
	public:
		EditorLayer(Scene* scene, Renderer* renderer);
		~EditorLayer();
		CameraComponent* GetSceneCamera();
		virtual void OnUpdate(double deltaTime) override;
		virtual void OnRender() override;
		virtual void OnImGuiRender() override;
		static void ResetDefaultLayout();
	private:
		void RenderDockSpace();
	};

}
