#include "pch.h"
#include "Application.h"
#include "CameraComponent.h"
#include "imgui/ImGuiScene.h"
#include "graphics/FrameBuffer.h"
#include "physics/Rigidbody.h"
#include "imgui/ImGuiUIEditor.h"

namespace Lobster
{

	CameraComponent* CameraComponent::s_activeCamera = nullptr;
    
    CameraComponent::CameraComponent() :
		Component(CAMERA_COMPONENT),
        m_fieldOfView(45.0f),
		m_width(1),
		m_height(1),
        m_nearPlane(0.1f),
        m_farPlane(100.0f),
        m_viewMatrix(glm::mat4(1.0f)),
        m_projectionMatrix(glm::mat4(1.0f))
    {
		glm::ivec2 size = Application::GetInstance()->GetWindowSize();
		m_width = size.x;
		m_height = size.y;
        ResizeProjection(m_width, m_height);
		m_frameBuffer = new FrameBuffer(m_width, m_height);
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

	void CameraComponent::ResizeProjection(float width, float height)
	{
		float aspectRatio = width / height;
		m_projectionMatrix = glm::perspectiveRH(glm::radians(m_fieldOfView), aspectRatio, m_nearPlane, m_farPlane);
		m_orthoMatrix = glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
		m_frustum.SetFromMatrix(m_projectionMatrix);
	}

	void CameraComponent::OnBegin() {
		if (!gameUI) return;
		for (Sprite2D* sprite : gameUI->GetSpriteList()) {
			sprite->OnBegin();
		}
	}
    
    void CameraComponent::OnUpdate(double deltaTime)
    {
		m_frustum.Update();
		//m_frustum.Draw(glm::inverse(m_viewMatrix));

#ifdef LOBSTER_BUILD_EDITOR
		// submit gizmos command
		GizmosCommand command;
		command.texture = "textures/ui/camera.png";
		command.position = transform->WorldPosition;
		command.source = gameObject;
		ImGuiScene::SubmitGizmos(command);
#endif
		DrawUI();
    }

	void CameraComponent::OnAttach()
	{
		if (s_activeCamera) {
			WARN("The scene already has an active camera, ignoring this camera...");
		}
		else {
			s_activeCamera = this;
		}

		PhysicsComponent* physics = new Rigidbody();
		physics->SetEnabled(false);
		gameObject->AddComponent(physics);
	}

	void CameraComponent::OnImGuiRender()
	{
		if (ImGui::CollapsingHeader("CameraComponent", ImGuiTreeNodeFlags_DefaultOpen))
		{
			// draw camera preview
			ImVec2 window_size = ImGui::GetItemRectSize();			
			void* image = m_frameBuffer->Get(0);
			float padding = 10.f;
			ImGui::Text("Camera Preview");
			ImVec2 imageSize = { (window_size.x - padding), (window_size.x - padding) / m_frameBuffer->GetAspectRatio() };
			ImGui::Image(image, imageSize, ImVec2(0, 1), ImVec2(1, 0));
			
			if (ImGui::Button("Edit UI")) {
				if (!gameUI) {
					gameUI = new GameUI();
				}
				b_uiEditor = true;
			}
			// properties control
			ImGui::PushItemWidth(80);
			ImGui::DragFloat("Near", &m_nearPlane, 0.1f, 0.01f, m_farPlane);
			ImGui::SameLine();
			ImGui::DragFloat("Far", &m_farPlane, 0.1f, m_nearPlane, 1000.f);
			ImGui::PopItemWidth();
			ResizeProjection(m_width, m_height);

			// show the UI editor
			if (b_uiEditor) {
				ImGuiUIEditor* editor = EditorLayer::GetUIEditor();
				editor->SetUI(gameUI);
				editor->Show(&b_uiEditor);
			}			
		}
	}

	void CameraComponent::DrawUI() {
		if (!gameUI) return;
		for (Sprite2D* sprite : gameUI->GetSpriteList()) {
			sprite->OnUpdate(0); // trivial input
			sprite->OnLateUpdate(0); // for callbacks
		}
	}

	void CameraComponent::Serialize(cereal::JSONOutputArchive & oarchive)
	{
		//LOG("Serializing CameraComponent");
		oarchive(*this);
	}

	void CameraComponent::Deserialize(cereal::JSONInputArchive & iarchive)
	{
		//LOG("Deserializing CameraComponent");
		try {
			iarchive(*this);
			glm::ivec2 size = Application::GetInstance()->GetWindowSize();
			ResizeProjection(size.x, size.y);
		}
		catch (std::exception e) {
			LOG("Deserializing CameraComponent failed, reason: {}", e.what());
		}
	}

	glm::mat4 CameraComponent::GetViewMatrix()
	{
		m_viewMatrix = glm::mat4_cast(glm::conjugate(transform->LocalRotation)) * glm::translate(-transform->WorldPosition);
		return m_viewMatrix;
	}
    
}
