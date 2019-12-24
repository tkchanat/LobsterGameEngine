#include "pch.h"
#include "physics/Rigidbody.h"
#include "objects/GameObject.h"

namespace Lobster {
	const glm::vec3 Rigidbody::GRAVITY = glm::vec3(0, -0.981, 0);

	void Rigidbody::OnUpdate(double deltaTime) {
		//	We should update but not draw the bounding box. Update and draw the collider according to user's option.
		m_boundingBox->OnUpdate(deltaTime);
		for (Collider* collider : m_colliders) {
			if (collider->IsEnabled()) {
				collider->OnUpdate(deltaTime);
				collider->DebugDraw();
			}
		}
	}

	void Rigidbody::OnAttach() {
		AABB* boundingBox = new AABB(this, Transform(), false);
		std::pair<glm::vec3, glm::vec3> pair = gameObject->GetBound();
		boundingBox->Min = pair.first;
		boundingBox->Max = pair.second;
		boundingBox->SetOwner(gameObject);
		boundingBox->SetOwnerTransform(transform);

		m_boundingBox = boundingBox;
	}

	void Rigidbody::OnImGuiRender() {
		bool statement;
		
		if (statement) {
			if (ImGui::CollapsingHeader("PhysicsComponent", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Checkbox("Enabled?", &m_enabled);
				ImGui::Checkbox("Simulate Physics", &m_simulate);
				ImGui::Combo("Physics Type", &m_physicsType, PhysicsType, 3);
			}
		}

		//	TODO: Confirmation Window.
		if (!m_show) {
			RemoveComponent(this);
		}

		//	Call colliders OnImGuiRender, by invoking base class OnImGuiRender.
		for (Collider* collider : m_colliders) {
			collider->OnImGuiRender();
		}
	}

	void Rigidbody::OnPhysicsUpdate(double deltaTime) {
		LOG("{} physics updated", gameObject->GetName());
		float time = (float)(deltaTime / 1000);
		if (m_simulate) {
			m_velocity += GRAVITY * time;
			transform->WorldPosition += m_velocity * time;
		}

	}

	//	TODO: Guess by taking half of deltaTime until we find a good approximation
	bool Rigidbody::WillCollide(double deltaTime) {
		return false;
	}
}