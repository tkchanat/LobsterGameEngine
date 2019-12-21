#include "pch.h"
#include "physics/Rigidbody.h"
#include "objects/GameObject.h"

namespace Lobster {
	const glm::vec3 Rigidbody::GRAVITY = glm::vec3(0, -9.81, 0);

	void Rigidbody::OnUpdate(double deltaTime) {
		//	We should update but not draw the bounding box. Update and draw the collider according to user's option.
		m_boundingBox->OnUpdate(deltaTime);
		for (Collider* collider : m_colliders) {
			if (collider->IsEnabled()) collider->OnUpdate(deltaTime);
		}

		if (m_enabled) {
			m_boundingBox->DebugDraw();
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
			ImGui::Checkbox("Enabled?", &m_enabled);
			ImGui::Combo("Physics Type", &m_physicsType, PhysicsType, 3);
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

	//	TODO: Guess by taking half of deltaTime until we find a good approximation
	bool Rigidbody::WillCollide(double deltaTime) {
		return false;
	}
}