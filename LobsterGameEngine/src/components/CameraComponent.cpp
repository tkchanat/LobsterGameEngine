#include "pch.h"
#include "Application.h"
#include "CameraComponent.h"
#include "imgui/ImGuiScene.h"
#include "graphics/FrameBuffer.h"
#include "physics/Rigidbody.h"

namespace Lobster
{

	CameraComponent* CameraComponent::s_activeCamera = nullptr;
    
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
		// unhook active camera
		if (s_activeCamera == this) {
			s_activeCamera = nullptr;
		}

		// release memory
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
#ifdef LOBSTER_BUILD_DEBUG
		// submit gizmos command
		GizmosCommand command;
		command.texture = "textures/ui/camera.png";
		command.position = transform->WorldPosition;
		command.source = gameObject;
		ImGuiScene::SubmitGizmos(command);
#endif
    }

	void CameraComponent::OnAttach()
	{
		if (s_activeCamera) {
			WARN("The scene already has an active camera, ignoring this camera...");
		}
		else {
			s_activeCamera = this;
		}

		Rigidbody* rigidbody = new Rigidbody();
		rigidbody->SetEnabled(false);
		gameObject->AddComponent(rigidbody);
	}

	void CameraComponent::OnImGuiRender()
	{
		if (ImGui::CollapsingHeader("CameraComponent", ImGuiTreeNodeFlags_DefaultOpen))
		{
			// draw camera preview
			ImVec2 window_size = ImGui::GetItemRectSize();
			void* image = m_frameBuffer->Get();
			ImGui::Text("Camera Preview");
			ImGui::Image(image, ImVec2(window_size.x, window_size.x / m_frameBuffer->GetAspectRatio()), ImVec2(0, 1), ImVec2(1, 0));
		}
	}

	void CameraComponent::Serialize(cereal::JSONOutputArchive & oarchive)
	{
		LOG("Serializing CameraComponent");
		oarchive(*this);
	}

	void CameraComponent::Deserialize(cereal::JSONInputArchive & iarchive)
	{
		LOG("Deserializing CameraComponent");
		try {
			iarchive(*this);
			float aspectRatio = Application::GetInstance()->GetWindowAspectRatio();
			ResizeProjection(aspectRatio);
		}
		catch (std::exception e) {
			LOG("Deserializing CameraComponent failed");
		}
	}

	glm::mat4 CameraComponent::GetViewMatrix() const
	{
		return glm::mat4_cast(glm::conjugate(transform->LocalRotation)) * glm::translate(-transform->WorldPosition);
	}
    
}
