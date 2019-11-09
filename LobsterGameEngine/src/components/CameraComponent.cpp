#include "pch.h"
#include "Application.h"
#include "CameraComponent.h"
#include "graphics/FrameBuffer.h"

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

		glm::ivec2 size = Application::GetInstance()->GetWindowSize();
		m_frameBuffer = new FrameBuffer(size.x, size.y);
    }
    
    CameraComponent::~CameraComponent()
    {
		if (m_frameBuffer) delete m_frameBuffer;
		m_frameBuffer = nullptr;
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
		if (ImGui::CollapsingHeader("CameraComponent", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::BeginChild("Camera Preview", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			{
				// draw camera preview
				ImGui::Text("Camera Preview");
				ImVec2 window_size = ImGui::GetWindowSize();
				void* image = m_frameBuffer->Get();
				ImGui::Image(image, ImVec2(window_size.x, window_size.x / m_frameBuffer->GetAspectRatio()), ImVec2(0, 1), ImVec2(1, 0));
			}
			ImGui::EndChild();
		}
	}

	glm::mat4 CameraComponent::GetViewMatrix() const
	{
		return glm::mat4_cast(glm::conjugate(transform->LocalRotation)) * glm::translate(-transform->WorldPosition);
	}
    
}
