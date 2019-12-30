#include "pch.h"
#include "physics/Rigidbody.h"
#include "objects/GameObject.h"

namespace Lobster {
	//	Sunny: This value is open to change.
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

				ImGui::DragFloat("Mass", &m_mass, 1.0f, 0.0001f, 100000.0f);

				ImGui::SliderFloat("Linear Damping", &m_linearDamping, 0.0f, 100.0f);
				ImGui::SliderFloat("Angular Damping", &m_angularDamping, 0.0f, 100.0f);

				ImGui::SliderFloat("Elasticity", &m_restitution, 0.0f, 1.0f);

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
		//	First obtain the amount of time elapsed in seconds. (deltaTime is originally in milliseconds)
		float time = (float)(deltaTime / 1000);

		//	Stores original information before this frame.
		m_prevLinearPos = transform->WorldPosition;
		m_prevAngularPos = transform->WorldEulerAngles;

		//	Copy velocity details from previous collision, if there's any.
		if (glm::length(m_newLinearVelocity) > 0) {
			m_velocity = m_newLinearVelocity;
			m_newLinearVelocity = glm::vec3(0, 0, 0);
		}

		glm::vec3 linearAccel = m_acceleration + (m_simulate ? GRAVITY : glm::vec3(0, 0, 0));
		//	Only update if simulation is enabled.
		if (m_simulate) {
			transform->WorldPosition += m_velocity * time + 0.5f * linearAccel * time;
			m_velocity += linearAccel * time;
			//	TODO: Damping algorithm
			m_velocity *= (100.0f - m_linearDamping) / 100.0f;

			transform->WorldEulerAngles += m_angularVelocity * time + 0.5f * m_angularAcceleration * time;
			m_angularVelocity += m_angularAcceleration * time;
			//	TODO: Damping algorithm
			m_angularVelocity *= (100.0f - m_angularDamping) / 100.0f;

			for (Collider* collider : m_colliders) {
				if (collider->IsEnabled()) {
					collider->OnUpdate(deltaTime);
				}
			}
		}
	}

	void Rigidbody::OnPhysicsLateUpdate(double deltaTime) {
		//	Now, check all possible objects for collision.
		//	Currently we don't have a good way to estimate. Therefore, we just check all :3
		std::vector<GameObject*> colliding = gameObject->GetColliding();

		//	Compare the list with that of the previous physics update.
		//	Call the corresponding functions.
		std::vector<GameObject*> collided = gameObject->GetCollided();

		//	If previous frame has no collision, call the two functions.
		for (GameObject* c : colliding) {
			if (std::find(collided.begin(), collided.end(), c) == collided.end()) {
				gameObject->OnEnter(c);
				c->OnEnter(gameObject);

				gameObject->OnCollide(c);
				c->OnCollide(gameObject);
			}
		}

		//	Call OnLeave if objects are not colliding at this frae, otherwise call OnOverlap.
		for (GameObject* c : collided) {
			if (std::find(colliding.begin(), colliding.end(), c) == colliding.end()) {
				gameObject->OnLeave(c);
				c->OnLeave(gameObject);
			}
			else {
				gameObject->OnOverlap(c);
				c->OnOverlap(gameObject);
			}
		}

		//	At last, update m_colliding and m_collided.
		gameObject->frameElapse();

		//	Update collision if this object is of block type.
		if (GetPhysicsType() != 0) return;

		for (GameObject* gameObj : colliding) {
			//	Update this object only if it is also of block type.
			Rigidbody* physicsObj = gameObj->GetComponent<Rigidbody>();
			if (physicsObj->GetPhysicsType() != 0) continue;
			if (!m_simulate && !(physicsObj->m_simulate)) continue;

			//	1. Compute normal of collision.
			glm::vec3 normal;
			glm::vec3 diff = physicsObj->transform->WorldPosition - transform->WorldPosition;
			if (abs(diff.x) > abs(diff.y) && abs(diff.x) > abs(diff.z)) {
				normal = glm::vec3(diff.x, 0, 0);
			} else if (abs(diff.y) > abs(diff.x) && abs(diff.y) > abs(diff.z)) {
				normal = glm::vec3(0, diff.y, 0);
			} else {
				normal = glm::vec3(0, 0, diff.z);
			}
			normal = glm::normalize(normal);

			//	2. Compute Impulse. We deferred the step of multiplying a constant to the next step.
			glm::vec3 impulse = -m_mass * physicsObj->m_mass * normal * glm::dot(normal, m_velocity - physicsObj->m_velocity);
			impulse /= (m_mass + physicsObj->m_mass);

			//	3. Update velocity.
			//	Different impulse and speed depending on whether the two objects simulate physics.
			if (!m_simulate) {
				impulse *= (1 + physicsObj->m_restitution);
				physicsObj->m_newLinearVelocity -= (impulse / m_mass + impulse / physicsObj->m_mass);
			} else if (!(physicsObj->m_simulate)) {
				impulse *= (1 + m_restitution);
				m_newLinearVelocity += impulse / m_mass + impulse / physicsObj->m_mass;
			} else {
				m_newLinearVelocity += impulse * (1 + m_restitution) / m_mass;
				physicsObj->m_newLinearVelocity -= impulse * (1 + physicsObj->m_restitution) / physicsObj->m_mass;
			}

			//	4. Update position to that before collision.
			transform->WorldPosition = m_prevLinearPos;
			physicsObj->transform->WorldPosition = physicsObj->m_prevLinearPos;
		}
	}
}