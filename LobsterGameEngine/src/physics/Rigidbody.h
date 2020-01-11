#pragma once
#include <glm/vec3.hpp>
#include "objects/Transform.h"
#include "physics/PhysicsComponent.h"

namespace Lobster {
	class Rigidbody : public PhysicsComponent {
	public:
		static const glm::vec3 GRAVITY;
		static const float RESISTANCE;

		Rigidbody() :
			m_velocity(glm::vec3(0, 0, 0)),
			m_acceleration(glm::vec3(0, 0, 0)),
			m_angularVelocity(glm::vec3(0, 0, 0)),
			m_angularAcceleration(glm::vec3(0, 0, 0)),
			m_prevLinearPos(glm::vec3(0, 0, 0)),
			m_prevAngularPos(glm::vec3(0, 0, 0)),
			m_newLinearVelocity(glm::vec3(0, 0, 0))
		{
		}
		virtual ~Rigidbody() override {
			m_lastCollided.clear();
		}

		void OnAttach() override;
		void OnUpdate(double deltaTime) override;
		void OnImGuiRender() override;
		void OnPhysicsUpdate(double deltaTime) override;
		void OnPhysicsLateUpdate(double deltaTime) override;

		//	Apply force and angular motion.
		//	position: Offset from COM position, in world coordinates (ie invariant with object scale and rotation)
		//	force: Force in Newton (N) to apply to the object.
		void ApplyForce(glm::vec3 position, glm::vec3 force);

	private:
		//	Velocity and acceleration for physics calculation.
		glm::vec3 m_velocity;
		glm::vec3 m_acceleration;

		//	Angular velocity and acceleration for physics calculation.
		glm::vec3 m_angularVelocity;
		glm::vec3 m_angularAcceleration;

		//	Variables used for position estimation.
		glm::vec3 m_prevLinearPos;
		glm::vec3 m_prevAngularPos;
		glm::vec3 m_newLinearVelocity;

		//	Used to store when is the last collsion.
		int lastCollision = 0;

		//	Damping factors. 0 means no damping.
		float m_linearDamping = 1.0f;
		float m_angularDamping = 1.0f;

		//	Coefficients of restitution. 0 means not bouncing, 1 means most elastic.
		float m_restitution = 0.0f;

		void Travel(float time, glm::vec3 linearAccel);
		void UndoTravel(float time, glm::vec3 linearAccel);
		std::vector<GameObject*> m_lastCollided;
	};
}