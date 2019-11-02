#include "pch.h"
#include "physics/Rigidbody.h"

namespace Lobster {
	const glm::vec3 Rigidbody::GRAVITY = glm::vec3(0, -9.81, 0);


	Rigidbody::Rigidbody(std::vector<glm::vec3> minMax) :
		PhysicsBody(minMax)
	{
		m_collider = new Collider(minMax);
	}

	void Rigidbody::OnUpdate(double deltaTime) {
		if (m_collider) m_collider->OnUpdate(deltaTime);
	}

	void Rigidbody::OnImGuiRender() {
		if (!m_enabled) return;

		if (ImGui::CollapsingHeader("Rigid body", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Checkbox("Gravity", &m_gravity);
		}

		if (m_collider) m_collider->OnImGuiRender();
	}

	//	TODO: Guess by taking half of deltaTime until we find a good approximation
	bool Rigidbody::WillCollide(double deltaTime) {
		return false;
	}
}