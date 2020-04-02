#pragma once

#include "GLFW/glfw3.h"     //  To be removed

namespace Lobster
{
    
	//	This class is for input polling, both keyboard input and mouse input.
	//	This class has a static interface, you may call it from anywhere.
    class Input
    {
    private:
		static bool m_locked;
        static glm::vec2 m_lastScroll;
		static glm::vec2 m_lastMouse;
		static glm::vec2 m_mouseDelta;
    public:
        //  Interface
		static void Initialize();
		static void Update();
		static void LockCursor();
		static void UnlockCursor();
		static bool InsideWindow(const glm::vec2& mouse, const glm::vec2& pos, const glm::vec2& size);
		static bool IsKeyUp(int key);
		static bool IsKeyDown(int key);
		static bool IsKeyHold(int key);
		static bool IsMouseUp(int button);
		static bool IsMouseDown(int button);
		static bool IsMouseHold(int button);		
		static void GetMousePos(double& x, double& y);
		static void GetWindowPos(int* x, int* y);
		static void GetWindowSize(int* x , int* y);
		static void ComputeCameraRay(glm::mat4 view, glm::mat4 proj, glm::vec3& origin, glm::vec3& direction,
			glm::vec2 window_pos = glm::vec2(-1, -1), glm::vec2 window_size = glm::vec2(-1, -1));
		static double GetMousePosX();
		static double GetMousePosY();
		static double GetMouseDeltaX();
		static double GetMouseDeltaY();
		inline static glm::vec2 GetLastScroll() { return m_lastScroll; }
		inline static glm::vec2 GetMouseDelta() { return m_mouseDelta; }
    };
    
}
