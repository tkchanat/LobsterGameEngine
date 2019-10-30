#include "pch.h"
#include "CameraComponent.h"

#include "Application.h"

namespace Lobster
{
    
    CameraComponent::CameraComponent(ProjectionType type) :
        m_fieldOfView(45.0f),
        m_nearPlane(0.1f),
        m_farPlane(100.0f),
        m_type(type),
        m_viewMatrix(glm::mat4(1.0f)),
        m_projectionMatrix(glm::mat4(1.0f))
    {
		float aspectRatio = Application::GetInstance()->GetWindowAspectRatio();
        ResizeProjection(aspectRatio);
    }
    
    CameraComponent::~CameraComponent()
    {
        
    }

	void CameraComponent::ResizeProjection(float aspectRatio)
	{
		switch (m_type) {
		case ProjectionType::PERSPECTIVE:
			m_projectionMatrix = glm::perspectiveRH(glm::radians(m_fieldOfView), aspectRatio, m_nearPlane, m_farPlane);
			break;
		case ProjectionType::ORTHOGONAL:
			//  TODO:   Orthogonal projection
			break;
		}
	}
    
    void CameraComponent::OnUpdate(double deltaTime)
    {
        
    }

	void CameraComponent::OnImGuiRender()
	{
	}

	std::vector<glm::vec3> CameraComponent::ObjectBound()
	{
		//	TODO: Fix the bound for the camera, I have no idea what I am writing over here. --sunny
		std::vector<glm::vec3> minMax(2);
		glm::vec3 pos = -transform->WorldPosition;
		minMax[0] = pos - glm::vec3(0.1, 0.1, 0.1);
		minMax[1] = pos + glm::vec3(0.1, 0.1, 0.1);
		return minMax;
	}

	glm::mat4 CameraComponent::GetViewMatrix() const
	{
		return glm::mat4_cast(glm::conjugate(transform->LocalRotation)) * glm::translate(-transform->WorldPosition);
	}
    
}
