#include "pch.h"
#include "FirstPersonController.h"

#include "system/Input.h"
#include "Application.h"

namespace Lobster
{
    
    FirstPersonController::FirstPersonController()
    {
		windowWidth = Application::GetInstance()->GetWindowSize().x;
		windowHeight = Application::GetInstance()->GetWindowSize().y;
        Input::GetMousePos(mouseLastX, mouseLastY);
        
        //  Set mouse wrap
        Window* window = Application::GetInstance()->GetWindow();
        //window->SetCursorHiddenWrap(true);
    }
    
    FirstPersonController::~FirstPersonController()
    {
        //  Disable mouse wrap
        Window* window = Application::GetInstance()->GetWindow();
        window->SetCursorHiddenWrap(false);
    }
    
    void FirstPersonController::OnUpdate(double deltaTime)
    {
        //  Movement
        glm::vec3 forward = transform->Forward() * Speed * (float)deltaTime;
        glm::vec3 strafe = transform->Right() * Speed * (float)deltaTime;
		if (Input::IsKeyDown(GLFW_KEY_W))
		{
			transform->Translate(-forward.x, -forward.y, -forward.z);
		}
		if (Input::IsKeyDown(GLFW_KEY_S))
		{
			transform->Translate(+forward.x, +forward.y, +forward.z);
		}
		if (Input::IsKeyDown(GLFW_KEY_A))
		{
			transform->Translate(-strafe.x, -strafe.y, -strafe.z);
		}
		if (Input::IsKeyDown(GLFW_KEY_D))
		{
			transform->Translate(+strafe.x, +strafe.y, +strafe.z);
		}
        
		//  Mouse Look
		double x, y;
		Input::GetMousePos(x, y);
		double velX = std::fmod((float)(x - mouseLastX), windowWidth);
		double velY = std::fmod((float)(y - mouseLastY), windowHeight);
		transform->RotateEuler((float)(-velY * Sensitivity * deltaTime), transform->Right());
		transform->RotateEuler((float)(-velX * Sensitivity * deltaTime), glm::vec3(0, 1, 0));
        
        mouseLastX = x;
        mouseLastY = y;
    }

	void FirstPersonController::OnImGuiRender()
	{
	}
    
}
