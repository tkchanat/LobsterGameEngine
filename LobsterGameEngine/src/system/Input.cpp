#include "pch.h"
#include "Input.h"
#include "events/EventCallback.h"
#include "events/EventDispatcher.h"

namespace Lobster
{
	bool Input::m_locked = false;
	glm::vec2 Input::m_lastScroll = { 0.0f, 0.0f };
	glm::vec2 Input::m_lastMouse = { 0.0f, 0.0f };
	glm::vec2 Input::m_mouseDelta = { 0.0f, 0.0f };

	void Input::Initialize()
	{
		EventDispatcher::AddCallback(EVENT_MOUSE_SCROLLED, new EventCallback<MouseScrolledEvent>([](MouseScrolledEvent* e) {
			m_lastScroll.x = e->dx;
			m_lastScroll.y = e->dy;
		}));
	}

	void Input::Update()
	{
		glfwPollEvents();

		// reset scroll
		m_lastScroll = glm::vec2(0.0f, 0.0f);		

		// calculate mouse delta
		double x, y;
		glfwGetCursorPos(Application::GetInstance()->GetWindow()->GetPtr(), &x, &y);
		m_mouseDelta = glm::vec2(x, y) - m_lastMouse;
		m_lastMouse = glm::vec2(x, y);

		// handle lock
		if (m_locked) {
			glfwSetCursorPos(Application::GetInstance()->GetWindow()->GetPtr(), 0, 0);
			m_lastMouse = glm::vec2(0, 0);
		}
	}

	void Input::LockCursor() {
		glfwSetCursorPos(Application::GetInstance()->GetWindow()->GetPtr(), 0, 0);
		m_lastMouse = glm::vec2(0, 0);
		m_locked = true;
	}

	void Input::UnlockCursor() {
		m_locked = false;
	}
	
	bool Input::InsideWindow(const glm::vec2& mouse, const glm::vec2& pos, const glm::vec2& size) {
		if (mouse.x >= pos.x && mouse.x <= pos.x + size.x && mouse.y >= pos.y && mouse.y <= pos.y + size.y)
			return true;
		return false;
	}

	bool Input::IsKeyUp(int key)
	{
		GLFWwindow* window = Application::GetInstance()->GetWindow()->GetPtr();
		int state = glfwGetKey(window, key);
		return state == GLFW_RELEASE;
	}

	bool Input::IsKeyDown(int key)
	{
		GLFWwindow* window = Application::GetInstance()->GetWindow()->GetPtr();
		int state = glfwGetKey(window, key);
		return state == GLFW_PRESS;
	}

	bool Input::IsKeyHold(int key)
	{
		GLFWwindow* window = Application::GetInstance()->GetWindow()->GetPtr();
		int state = glfwGetKey(window, key);
		return state == GLFW_REPEAT;
	}

	bool Input::IsMouseUp(int button)
	{
		GLFWwindow* window = Application::GetInstance()->GetWindow()->GetPtr();
		int state = glfwGetMouseButton(window, button);
		return state == GLFW_RELEASE;
	}

	bool Input::IsMouseDown(int button)
	{
		GLFWwindow* window = Application::GetInstance()->GetWindow()->GetPtr();
		int state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
	}

	bool Input::IsMouseHold(int button)
	{
		GLFWwindow* window = Application::GetInstance()->GetWindow()->GetPtr();
		int state = glfwGetMouseButton(window, button);
		return state == GLFW_REPEAT;
	}

	void Input::GetMousePos(double& x, double& y)
	{
		GLFWwindow* window = Application::GetInstance()->GetWindow()->GetPtr();
		glfwGetCursorPos(window, &x, &y);
		// non-game mode: calculate the position to that of the game window
		#if LOBSTER_BUILD_DEBUG
		{
			int winPosX, winPosY, winSizeX, winSizeY;
			GetWindowPos(&winPosX, &winPosY);
			GetWindowSize(&winSizeX, &winSizeY);
			Config& config = Application::GetInstance()->GetConfig();
			glm::vec2 tabPos = config.gameTabPos;
			glm::vec2 tabSize = config.gameTabSize;
			// calculate the difference in y caused by the change in aspect ratio
			float actualTabSizeY = tabSize.x * winSizeY / winSizeX;
			float aspectRatioDiffY = (tabSize.y - actualTabSizeY) * 0.5;

			float scaleX = tabSize.x / winSizeX, scaleY = actualTabSizeY / winSizeY;
			float transX = tabPos.x - winPosX, transY = tabPos.y - winPosY;
			x = (x - transX) / scaleX;
			y = (y - transY - aspectRatioDiffY) / scaleY;
		}
		#endif
	}

	void Input::GetWindowPos(int* x, int* y) {
		GLFWwindow* window = Application::GetInstance()->GetWindow()->GetPtr();
		glfwGetWindowPos(window, x, y);
	}

	void Input::GetWindowSize(int* x, int* y) {
		GLFWwindow* window = Application::GetInstance()->GetWindow()->GetPtr();
		glfwGetWindowSize(window, x, y);
	}

	// Modified version from ImGuizmo::ComputeCameraRay()	
	void Input::ComputeCameraRay(glm::mat4 view, glm::mat4 proj, glm::vec3& origin, glm::vec3& direction,
		glm::vec2 window_pos, glm::vec2 window_size)
	{
		ImGuiIO& io = ImGui::GetIO();
		// if window pos and window size are not input, default to be the size and position of the glfw window
		if (window_pos[0] < 0 || window_pos[1] < 0) {
			int wx, wy;
			GetWindowPos(&wx, &wy);
			window_pos = glm::vec2(wx, wy);
		}
		if (window_size[0] < 0 || window_size[1] < 0) {
			int wx, wy;
			GetWindowSize(&wx, &wy);
			window_size = glm::vec2(wx, wy);
		}
		glm::mat4 viewProjInv = glm::inverse(proj * view);
		float mox = ((io.MousePos.x - window_pos.x) / window_size.x) * 2.f - 1.f;
		float moy = (1.f - ((io.MousePos.y - window_pos.y) / window_size.y)) * 2.f - 1.f;

		glm::vec4 rayOrigin = viewProjInv * glm::vec4(mox, moy, 0.f, 1.f);
		rayOrigin /= rayOrigin.w;
		origin = rayOrigin;
		glm::vec4 rayEnd = viewProjInv * glm::vec4(mox, moy, 1.f, 1.f);
		rayEnd /= rayEnd.w;
		direction = glm::normalize(glm::vec3(rayEnd - rayOrigin));
	}

	double Input::GetMousePosX() {
		GLFWwindow* window = Application::GetInstance()->GetWindow()->GetPtr();
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		return x;
	}

	double Input::GetMousePosY() {
		GLFWwindow* window = Application::GetInstance()->GetWindow()->GetPtr();
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		return y;
	}

	double Input::GetMouseDeltaX() {
		return m_mouseDelta.x;
	}

	double Input::GetMouseDeltaY() {
		return m_mouseDelta.y;
	}
}
