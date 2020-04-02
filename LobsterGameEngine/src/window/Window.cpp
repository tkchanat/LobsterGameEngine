#include "pch.h"
#include "Window.h"
#include "events/EventCollection.h"
#include "events/EventDispatcher.h"
#include "events/EventQueue.h"
#include "stb_image.h"

namespace Lobster
{

	Window::Window(int width, int height, std::string title, bool vsync) :
		m_window(nullptr),
        m_size(glm::ivec2(width, height)),
        m_frameBufferSize(glm::ivec2(width, height)),
        m_title(title),
		b_isRunning(false),
		b_vsync(vsync)
    {
		Initialize();
    }
    
    Window::~Window()
    {
        glfwTerminate();
    }

	void Window::Swap() const
    {
        glfwSwapBuffers(m_window);
    }
    
    void Window::SetCursorHiddenWrap(bool wrap)
    {
        if(wrap)
        {
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else
        {
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    
	void Window::Initialize()
	{
		/* Initialize the library */
		if (!glfwInit())
		{
			throw std::runtime_error("GLFW failed to initialize!");
		}

		/* GLFW Window Hint and Configure OpenGL Version */
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

		/* Create a windowed mode window and its OpenGL context */
		m_window = glfwCreateWindow(m_size.x, m_size.y, m_title.c_str(), NULL, NULL);
		if (!m_window)
		{
			glfwTerminate();
			throw std::runtime_error("GLFW context window failed to create! Check if your machine supports up to OpenGL 4.1");
		}

		glfwMakeContextCurrent(m_window);	//	Make window's context current
		glfwSwapInterval((int)b_vsync);	//	Enable VSync

		GLFWimage image; 
		std::string iconPath = "../lobster.png";
		if (!iconPath.empty())
		{
			std::string path = FileSystem::Path(iconPath);
			image.pixels = stbi_load(path.c_str(), &image.width, &image.height, 0, 4); //rgba channels 
			glfwSetWindowIcon(m_window, 1, &image);
			stbi_image_free(image.pixels);
		}

#ifdef LOBSTER_PLATFORM_WIN
		if (!gladLoadGL())
		{
			throw std::runtime_error("GLAD initialization failed! Check gladLoadGL() error code for diagnoses");
			exit(-1);
		}
#endif

		// get frame buffer size (required reason: system-dependent)
		glfwGetFramebufferSize(m_window, &m_size.x, &m_size.y);


		/* Listen for GLFW Callback */
		glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			switch (action) {
			case GLFW_PRESS:
				EventQueue::GetInstance()->AddEvent<KeyPressedEvent>(key, false, mods);
				break;
			case GLFW_RELEASE:
				EventQueue::GetInstance()->AddEvent<KeyReleasedEvent>(key);
				break;
			case GLFW_REPEAT:
				EventQueue::GetInstance()->AddEvent<KeyPressedEvent>(key, true, mods);
				break;
			}
		});
		glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods) {
			switch (action) {
			case GLFW_PRESS:
				EventQueue::GetInstance()->AddEvent<MousePressedEvent>(button, false);
				break;
			case GLFW_RELEASE:
				EventQueue::GetInstance()->AddEvent<MouseReleasedEvent>(button);
				break;
			case GLFW_REPEAT:
				EventQueue::GetInstance()->AddEvent<MousePressedEvent>(button, true);
				break;
			}
		});
		glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xOffset, double yOffset) {
			EventQueue::GetInstance()->AddEvent<MouseScrolledEvent>((float)xOffset, (float)yOffset);
		});
		glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window) {
			EventQueue::GetInstance()->AddEvent<WindowClosedEvent>();
		});
		glfwSetWindowFocusCallback(m_window, [](GLFWwindow* window, int focused) {
			EventQueue::GetInstance()->AddEvent<WindowFocusedEvent>((bool)focused);
		});

		/* Listen for Lobster Callback */
		//EventDispatcher::AddCallback(EVENT_KEY_PRESSED, new EventCallback<KeyPressedEvent>([this](KeyPressedEvent* e) {
		//	if (e->Key == GLFW_KEY_ESCAPE)
		//	{
		//		b_isRunning = false;
		//	}
		//}));
		EventDispatcher::AddCallback(EVENT_WINDOW_CLOSED, new EventCallback<WindowClosedEvent>([this](WindowClosedEvent* e) {
			b_isRunning = false;
		}));

		b_isRunning = true;
	}
    
}
