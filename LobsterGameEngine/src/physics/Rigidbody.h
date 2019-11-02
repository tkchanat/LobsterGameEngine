#pragma once
#include <glm/vec3.hpp>
#include "objects/Transform.h"
#include "physics/PhysicsBody.h"

namespace Lobster {
	class Rigidbody : public PhysicsBody {
	public:
		static const glm::vec3 GRAVITY;

		Rigidbody(std::vector<glm::vec3> minMax);
		void OnUpdate(double deltaTime) override;
		void OnImGuiRender() override;

		inline ColliderType GetColliderType() const { return m_collider ? m_collider->GetColliderType() : ColliderType::BOUND; }
		bool WillCollide(double deltaTime);

	private:
		glm::vec3 m_velocity;
		glm::vec3 m_acceleration;

		bool m_gravity = false;
	};
}