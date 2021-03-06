#pragma once
#include "Layer.h"

namespace Lobster
{

	class Scene;
	class Renderer;
	class CameraComponent;
	class GameObject;
	class ImGuiMenuBar;
	class ImGuiScene;
	class ImGuiDemoWindow;
	class ImGuiComponent;
	class ImGuiGame;
	class ImGuiUIEditor;

	class EditorLayer : public Layer
	{
	public:
		static GameObject* s_selectedGameObject;
		static ImGuiID centerID;		
	private:
		static uint s_dockspace_id;
		static ImGuiUIEditor* s_uiEditor;
		ImGuiMenuBar* m_menuBar;
		ImGuiScene* m_scene;
		ImGuiComponent* m_toolbar;
		ImGuiComponent* m_properties;
		ImGuiComponent* m_console;
		ImGuiComponent* m_assets;
		ImGuiComponent* m_hierarchy;		
		ImGuiDemoWindow* m_demoWindow;
	public:
		EditorLayer();
		~EditorLayer();
		CameraComponent* GetSceneCamera();		
		virtual void OnUpdate(double deltaTime) override;
		virtual void OnRender() override;
		virtual void OnImGuiRender() override;
		static void ResetDefaultLayout();
		inline static ImGuiUIEditor* GetUIEditor() { return s_uiEditor; }
	private:
		void RenderDockSpace();
	};

}
