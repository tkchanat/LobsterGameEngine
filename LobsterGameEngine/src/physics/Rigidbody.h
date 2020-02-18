#pragma once
#include <glm/vec3.hpp>
#include "objects/Transform.h"
#include "physics/PhysicsComponent.h"

namespace Lobster {
	class Rigidbody : public PhysicsComponent {
	public:
		static const glm::vec3 GRAVITY;
		static const float RESISTANCE;

		Rigidbody() : m_prevRotation(glm::vec3(0, 0, 0)) {}

		void OnAttach() override;
		void OnUpdate(double deltaTime) override;
		void OnImGuiRender() override;
		void OnPhysicsUpdate(double deltaTime) override;

	private:
		int m_isChanging = -1;		//	Used for undo system. 0 = mass, 1 = linear damping, 2 = angular damping, 3 = elasticity.
		float m_prevProp[4];		//	Used for undo system.

		//	Variables used for position estimation.
		glm::vec3 m_prevLinearPos;
		glm::vec3 m_prevAngularPos;
		glm::vec3 m_newLinearVelocity;

		//	Used to store when is the last collsion.
		int lastCollision = 0;

		//	Used to store prev rotation details. (NEW)
		glm::vec3 m_prevRotation;

		//	Damping factors. 0 means no damping.
		//	The factor is a value from 0 to 100. We will reduce acceleration by (damping factor)% each second.
		float m_linearDamping = 1.0f;
		float m_angularDamping = 1.0f;

		//	Coefficients of restitution. 0 means not bouncing, 1 means most elastic.
		float m_restitution = 0.0f;

		//	Find the normal between this and other. Normal is pointing out from this, towards other direction.
		glm::vec3 GetNormal(Rigidbody* other) const;

		//	Let the object travel in given time (in milliseconds).
		void Travel(float time, bool gravity = true, bool damping = true);

		//	Undo the object's motion.
		void UndoTravel(float time);
	};
}