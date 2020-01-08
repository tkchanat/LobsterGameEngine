#pragma once
#include "layer/LayerStack.h"
#include "window/Window.h"

namespace Lobster
{

    class Renderer;
    class Scene;
	class GUILayer;
	class EditorLayer;
	class FileSystem;
	class UndoSystem;

	enum ApplicationMode {
		EDITOR, SIMULATION, GAME
	};
    
    class Application
    {
		friend class ImGuiComponent;
    private:
		static ApplicationMode mode;	// application mode (editor, simulation, game)
        static Application* m_instance;
        Window* m_window;
        Renderer* m_renderer;
        Scene* m_scene;
		FileSystem* m_fileSystem;
		UndoSystem* m_undoSystem;
		std::string originalTitle;
		int m_targetFPS = 60;
		int m_maxFixedUpdates = 10;
		// Layers
		//LayerStack m_layerStack;
		EditorLayer* m_editorLayer; // depends on GUILayer
		GUILayer* m_GUILayer;
		void FixedUpdate(double deltaTime);
		void VariableUpdate(double deltaTime);		

    public:
        Application();
        ~Application();
        void Initialize();
        void Run();
        void Shutdown();
		void OpenScene(const char* scenePath);
        inline Window* GetWindow() { return m_window; }
        inline glm::ivec2 GetWindowSize() { return m_window->GetSize(); }
        inline float GetWindowAspectRatio() { return (float)m_window->GetSize().x / (float)m_window->GetSize().y; }
        inline static Application* GetInstance() { return m_instance; }	
		// Can interchange between editor mode and simulation mode
		static void SwitchMode(ApplicationMode mode);
		inline static ApplicationMode GetMode() { return mode; }
    };
    
}
