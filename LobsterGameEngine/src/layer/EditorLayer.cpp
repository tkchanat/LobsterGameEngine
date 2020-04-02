#include "pch.h"
#include "EditorLayer.h"

#include "events/EventQueue.h"
#include "graphics/Scene.h"
#include "graphics/Renderer.h"
#include "objects/GameObject.h"
#include "window/Window.h"

#include "imgui/ImGuiAssets.h"
#include "imgui/ImGuiConsole.h"
#include "imgui/ImGuiDemoWindow.h"
#include "imgui/ImGuiHierarchy.h"
#include "imgui/ImGuiMenuBar.h"
#include "imgui/ImGuiProperties.h"
#include "imgui/ImGuiScene.h"
#include "imgui/ImGuiToolbar.h"
#include "imgui/ImGuiUIEditor.h"

#include <imgui_internal.h>

namespace Lobster
{
	// Static initialization
	uint EditorLayer::s_dockspace_id = 0; 
	GameObject* EditorLayer::s_selectedGameObject = nullptr;
	ImGuiUIEditor* EditorLayer::s_uiEditor = nullptr;
	ImGuiScene* ImGuiScene::s_instance = nullptr;
	SSFLog ImGuiConsole::log;

	EditorLayer::EditorLayer() :
		Layer("Game Layer"),
		m_assets(new ImGuiAssets()),
		m_console(new ImGuiConsole()),
		m_demoWindow(new ImGuiDemoWindow()),
		m_hierarchy(new ImGuiHierarchy()),
		m_menuBar(new ImGuiMenuBar()),
		m_properties(new ImGuiProperties()),
		m_scene(new ImGuiScene()),		
		m_toolbar(new ImGuiToolbar())
	{
		s_uiEditor = new ImGuiUIEditor(); // created but not shown
		// configure imgui io setting
		ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;
	}

	EditorLayer::~EditorLayer()
	{
	}

	CameraComponent * EditorLayer::GetSceneCamera()
	{
		return m_scene->GetCamera();
	}

	void EditorLayer::OnUpdate(double deltaTime)
	{
		m_scene->OnUpdate(deltaTime);
	}

	void EditorLayer::OnRender()
	{
	}

	void EditorLayer::OnImGuiRender()
	{
		RenderDockSpace();
		static bool show_assets = true;
		static bool show_console = true;
		static bool show_demo = true;
		static bool show_hierarchy = true;
		static bool show_menuBar = true;
		static bool show_properties = true;
		static bool show_scene = true;
		static bool show_toolbar = true;
		m_assets->Show(&show_assets); // Assets
		m_console->Show(&show_console); // Console
		m_demoWindow->Show(&show_demo); // Demo window
		m_hierarchy->Show(&show_hierarchy); // Hierarchy
		m_menuBar->Show(&show_menuBar); // Menu bar
		m_scene->Show(&show_scene); // Scene
		m_properties->Show(&show_properties); // Properties
		m_toolbar->Show(&show_toolbar); // Toolbar	
		ImGui::End();
	}

	void EditorLayer::RenderDockSpace()
	{
		// DockSpace ==========================================================================================
		static bool show_dockspace = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		s_dockspace_id = ImGui::GetID("MyDockSpace"); // update dock space id

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace", &show_dockspace, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		ImGuiIO& io = ImGui::GetIO();
		if (ImGui::DockBuilderGetNode(s_dockspace_id) == NULL)
		{
			// Initialize default layout if layout was not set
			EditorLayer::ResetDefaultLayout();
		}

		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGui::DockSpace(s_dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}		
	}

	void EditorLayer::ResetDefaultLayout()
	{
		//	Setting up default docking space layout
		ImVec2 dockspace_size = ImGui::GetMainViewport()->Size;
		ImGui::DockBuilderRemoveNode(s_dockspace_id); // Clear out existing layout
		ImGui::DockBuilderAddNode(s_dockspace_id, ImGuiDockNodeFlags_None); // Add empty node
		ImGui::DockBuilderSetNodeSize(s_dockspace_id, dockspace_size);

		ImGuiID dock_main_id = s_dockspace_id; // This variable will track the document node, however we are not using it here as we aren't docking anything into it.		
		// ============================================================================ //
		//										0w01									//
		// ============================================================================ //
		//			0314		|								|						//
		//			    		|								|			    		//
		// =====================|				3718			|						//
		//	        0348		|								|           7w0h        //
		//						|								|						//
		// =====================================================|			    		//
		//							078h						|						//
		// ============================================================================ //
		// Naming scheme: dock_<left><right><top><bottom>, 0 = leftmost, w/h(10) = rightmost/downmost
		ImGuiID dock_0w01, dock_0w1h, dock_070h, dock_7w0h, dock_0708, dock_078h, dock_0318, dock_3718, dock_0314, dock_0348;		
		// Disable tab bar for custom toolbar
		ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Up, 0.06f, &dock_0w01, &dock_0w1h);
		ImGui::DockBuilderGetNode(dock_0w01)->LocalFlags |= ImGuiDockNodeFlags_AutoHideTabBar;
		ImGui::DockBuilderSplitNode(dock_0w1h, ImGuiDir_Left, 0.7f, &dock_070h, &dock_7w0h);
		ImGui::DockBuilderSplitNode(dock_070h, ImGuiDir_Up, 0.8f, &dock_0708, &dock_078h);
		ImGui::DockBuilderSplitNode(dock_0708, ImGuiDir_Left, 0.3f, &dock_0318, &dock_3718);
		ImGui::DockBuilderSplitNode(dock_0318, ImGuiDir_Up, 0.4f, &dock_0314, &dock_0348);
		ImGui::DockBuilderDockWindow("Toolbar", dock_0w01);
		ImGui::DockBuilderDockWindow("Scene", dock_3718);
		ImGui::DockBuilderDockWindow("Editor UI", dock_3718); // dont remove, it fixes bug
		ImGui::DockBuilderDockWindow("Assets", dock_078h);
		ImGui::DockBuilderDockWindow("Console", dock_078h);
		ImGui::DockBuilderDockWindow("Hierarchy", dock_0314);
		ImGui::DockBuilderDockWindow("Dear ImGui Demo", dock_0348);
		ImGui::DockBuilderDockWindow("Properties", dock_7w0h);		
		ImGui::DockBuilderFinish(s_dockspace_id);
		// set the center ID
		centerID = dock_3718;
	}

	ImGuiID EditorLayer::centerID = 0;
}
