#include "pch.h"
#include "Application.h"
#include "Input.h"
#include "events/EventCallback.h"
#include "events/EventDispatcher.h"

namespace Lobster
{

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
	}

}
