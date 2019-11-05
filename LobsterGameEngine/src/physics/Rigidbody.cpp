#include "pch.h"
#include "physics/Rigidbody.h"
#include "objects/GameObject.h"

namespace Lobster {
	const glm::vec3 Rigidbody::GRAVITY = glm::vec3(0, -9.81, 0);

	void Rigidbody::OnUpdate(double deltaTime) {
		//	We should update but not draw the bounding box. Update and draw the collider according to user's option.
		m_boundingBox->OnUpdate(deltaTime);
	}

	void Rigidbody::OnImGuiRender() {
		bool statement;
		if (gameObject->GetColliders().size() == 0) {
			statement = ImGui::CollapsingHeader("Rigidbody", &m_show, ImGuiTreeNodeFlags_DefaultOpen);
		} else {
			statement = ImGui::CollapsingHeader("Rigidbody", ImGuiTreeNodeFlags_DefaultOpen);
		}
		
		if (statement) {
			ImGui::Checkbox("Enabled?", &m_enabled);
			ImGui::Combo("Physics Type", &m_physicsType, PhysicsType, 3);
		}

		//	TODO: Confirmation Window.
		if (!m_show) {
			RemoveComponent(this);
		}
	}

	//	TODO: Guess by taking half of deltaTime until we find a good approximation
	bool Rigidbody::WillCollide(double deltaTime) {
		return false;
	}
}