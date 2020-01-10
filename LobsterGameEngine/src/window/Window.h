#pragma once
#include <GLFW/glfw3.h>

namespace Lobster
{

	//	This class is responsible for encapsulating the window creation and all window events handling.
	//	Currently, we are using GLFW as a kickstarter / driver for our project.
	//	GLFW is awesome, check it out here: https://github.com/glfw/glfw
	//	You may consider implementing it in the future.
	//	Also, this class is should be platform independent. Please implement a platform dependent class by inheriting this class.
    class Window
	{
    private:
        GLFWwindow* m_window;
        glm::ivec2 m_size;
        glm::ivec2 m_frameBufferSize;
        std::string m_title;
        bool b_isRunning;
		bool b_vsync;
	public:
        Window(int width, int height, std::string title, bool vsync);
		~Window();
        void Swap() const;
        void SetCursorHiddenWrap(bool wrap);
        inline glm::ivec2 GetSize() const { return m_size; }
        inline glm::ivec2 GetFrameBufferSize() const { return m_frameBufferSize; }
        inline std::string GetTitle() const { return m_title; }
        inline void SetTitle(const char* title) { glfwSetWindowTitle(m_window, title); }
        inline bool IsRunning() const { return b_isRunning; }
        inline GLFWwindow* GetPtr() { return m_window; }
	private:
		void Initialize();
	};

}
