#include "pch.h"
#include "physics/Rigidbody.h"

namespace Lobster {
	const glm::vec3 Rigidbody::GRAVITY = glm::vec3(0, -9.81, 0);


	Rigidbody::Rigidbody(std::vector<glm::vec3> minMax) :
		PhysicsComponent(minMax)
	{
		m_boundingBox = new AABB(minMax, false);
		m_collider = new AABB(minMax);
	}

	void Rigidbody::OnUpdate(double deltaTime) {
		//	We should update but not draw the bounding box. Update and draw the collider according to user's option.
		m_boundingBox->OnUpdate(transform);
		m_collider->OnUpdate(transform);
	}

	void Rigidbody::OnImGuiRender() {
		ImGui::PushID(this);
		if (ImGui::CollapsingHeader("Physics Collider", &m_show, ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Checkbox("Enabled?", &m_enabled);

			ImGui::Combo("Physics Type", &m_physicsType, PhysicsType, 3);
			ImGui::Combo("Collider Type", &m_colliderType, ColliderType, 2);

			//	TODO: When we add support for SphereCollider, add and implement this code to implement switching between Sphere / Box Collider.
			//	if (ImGui::IsItemActive());

			m_collider->OnImGuiRender();
		}
		ImGui::PopID();

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