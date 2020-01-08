#pragma once
#include <glm/vec3.hpp>
#include "objects/Transform.h"
#include "physics/PhysicsComponent.h"

namespace Lobster {
	class Rigidbody : public PhysicsComponent {
	public:
		static const glm::vec3 GRAVITY;

		Rigidbody() : m_velocity(glm::vec3(0, 0, 0)), m_acceleration(glm::vec3(0, 0, 0)) {}
		virtual ~Rigidbody() override {}

		void OnAttach() override;
		void OnUpdate(double deltaTime) override;
		void OnImGuiRender() override;

		bool WillCollide(double deltaTime);

	private:
		glm::vec3 m_velocity;
		glm::vec3 m_acceleration;

		bool m_gravity = false;
	};
}