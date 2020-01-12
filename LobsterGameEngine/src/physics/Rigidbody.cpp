#include "pch.h"
#include "physics/Rigidbody.h"
#include "objects/GameObject.h"

namespace Lobster {
	//	Sunny: This value is open to change.
	const glm::vec3 Rigidbody::GRAVITY = glm::vec3(0, -0.981, 0);
	const float Rigidbody::RESISTANCE = 0.001f;

	void Rigidbody::OnUpdate(double deltaTime) {
		//	We should update but not draw the bounding box. Update and draw the collider according to user's option.
		m_boundingBox->OnUpdate(deltaTime);
		for (Collider* collider : m_colliders) {
			if (collider->IsEnabled()) {
				collider->OnUpdate(deltaTime);
				collider->Draw();
			}
		}
	}

	void Rigidbody::OnAttach() {
		AABB* boundingBox = new AABB(this, Transform());
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

		//	Compute acceleration due to gravity.
		glm::vec3 linearAccel = m_acceleration + (m_simulate ? GRAVITY : glm::vec3(0, 0, 0));

		//	Determine if we need to update position or velocity for each direction.

		for (int i = 0; i < 3; i++) {
			if (m_newLinearVelocity[i] * m_velocity[i] < 0.0f && m_newLinearVelocity[i] + m_velocity[i] < 0.01f) {
				m_velocity[i] = 0.0f;
				m_newLinearVelocity[i] = 0.0f;
				linearAccel[i] = 0.0f;
				lastCollision = 0;
			}
			else {
				lastCollision++;
			}
		}

		//	Copy velocity details from previous collision, if there's any.
		if (glm::length(m_newLinearVelocity) > 0) {
			m_velocity = m_newLinearVelocity;
			m_newLinearVelocity = glm::vec3(0, 0, 0);
		}

		//	Only update if simulation is enabled.
		if (m_simulate) {

			Travel(time, linearAccel);

			for (Collider* collider : m_colliders) {
				if (collider->IsEnabled()) {
					collider->OnUpdate(deltaTime);
				}
			}

			if (lastCollision > 5) return;

			for (GameObject* collided : gameObject->GetLastCollided()) {
				PhysicsComponent* physics = collided->GetComponent<PhysicsComponent>();
				if (Intersects(physics)) {
					UndoTravel(time, linearAccel);

					for (Collider* collider : m_colliders) {
						if (collider->IsEnabled()) {
							collider->OnUpdate(deltaTime);
						}
					}

					lastCollision = 0;
					break;
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

		//	Copy vector of most recently collided GameObject.
		m_lastCollided.empty();
		if (colliding.size() > 0) {
			m_lastCollided = colliding;
		}

		//	If previous frame has no collision, call the two functions.
		for (GameObject* c : colliding) {
			if (std::find(collided.begin(), collided.end(), c) == collided.end()) {
				gameObject->OnEnter(c);

				gameObject->OnCollide(c);
			}
		}

		//	Call OnLeave if objects are not colliding at this frae, otherwise call OnOverlap.
		for (GameObject* c : collided) {
			if (std::find(colliding.begin(), colliding.end(), c) == colliding.end()) {
				gameObject->OnLeave(c);
			}
			else {
				gameObject->OnOverlap(c);
			}
		}

		//	Update collision if this object is of block type.
		if (GetPhysicsType() != 0) return;

		//	Now, resolve collision event between this game object and the other game object (gameObj).
		for (GameObject* gameObj : colliding) {
			//	Update this object only if it is also of block type.
			Rigidbody* physicsObj = gameObj->GetComponent<Rigidbody>();
			if (physicsObj->GetPhysicsType() != 0) continue;
			if (!m_simulate && !(physicsObj->m_simulate)) continue;

			//	1. Compute normal of collision.
			glm::vec3 normal = GetNormal(physicsObj);
			glm::vec3 diff = physicsObj->transform->WorldPosition - transform->WorldPosition;

			//	2. Determine if the Center of Mass of the object falls inside the opposing collider.
			glm::vec3 normalProjection = glm::translate(physicsObj->m_centerOfMass) * glm::vec4(gameObj->transform.WorldPosition, 1.0f);
			normalProjection -= glm::dot(diff, normal) * normal;

			//	Try to map this object into gameObj coordinate system.
			glm::vec3 mappedPosition = glm::inverse(transform->GetMatrix()) * glm::vec4(normalProjection, 1.0f);
			std::pair<glm::vec3, glm::vec3> pairBound = GetOwner()->GetBound();

			glm::vec3 displacement = glm::vec3(0, 0, 0);
			bool centerOutOfObject = false;

			//	Check if this object lies outside gameObj by checking trasformed bounding box.
			if (pairBound.first.x > mappedPosition.x) {
				displacement.x -= pairBound.first.x - mappedPosition.x;
				centerOutOfObject = true;
			} else if (pairBound.second.x < mappedPosition.x) {
				displacement.x += mappedPosition.x - pairBound.second.x;
				centerOutOfObject = true;
			}

			if (pairBound.first.y > mappedPosition.y) {
				displacement.y -= pairBound.first.y - mappedPosition.y;
				centerOutOfObject = true;
			} else if (pairBound.second.y < mappedPosition.y) {
				displacement.y += mappedPosition.y - pairBound.second.y;
				centerOutOfObject = true;
			}

			if (pairBound.first.z > mappedPosition.z) {
				displacement.z -= pairBound.first.z - mappedPosition.z;
				centerOutOfObject = true;
			} else if (pairBound.second.z < mappedPosition.z) {
				displacement.z += mappedPosition.z - pairBound.second.z;
				centerOutOfObject = true;
			}
		
			//	3. Compute Impulse. We deferred the step of multiplying a constant to the next step.
			glm::vec3 impulse = -m_mass * physicsObj->m_mass * normal * glm::dot(normal, m_velocity - physicsObj->m_velocity);
			impulse /= (m_mass + physicsObj->m_mass);

			//	4. Apply force due to lost of balance, with our newly calculated impulse.
			//	Case 1 - Object COM out of another object. (centerOutOfObject flag)
			//	Case 2 - Corresponding position of COM is not lying on the ground. (else case)
			if (centerOutOfObject && physicsObj->m_simulate && collided.size() == 0) {
				//	Case 1
				physicsObj->ApplyForce(displacement, impulse);
			}
			//} else {
			//	glm::vec3 oppositeNormal = physicsObj->GetNormal(this);

			//	//	Case 2
			//	glm::vec3 angularForce = glm::cross(normal, oppositeNormal);
			//	if (glm::length(angularForce) > 0.01f) {
			//		m_angularVelocity += angularForce;
			//	}
			//}

			//	5. Update velocity.
			//	Different impulse and speed depending on whether the two objects simulate physics.
			if (!m_simulate) {
				impulse *= (1 + physicsObj->m_restitution);
				physicsObj->m_newLinearVelocity -= impulse / physicsObj->m_mass + impulse / m_mass;
			} else if (!(physicsObj->m_simulate)) {
				impulse *= (1 + m_restitution);
				m_newLinearVelocity += impulse / m_mass + impulse / physicsObj->m_mass;
			} else {
				m_newLinearVelocity += impulse * (1 + m_restitution) / m_mass;
				physicsObj->m_newLinearVelocity -= impulse * (1 + physicsObj->m_restitution) / physicsObj->m_mass;
			}

			//	6. Update position to that before collision.
			transform->WorldPosition = m_prevLinearPos;
			physicsObj->transform->WorldPosition = physicsObj->m_prevLinearPos;
		}
	}

	void Rigidbody::ApplyForce(glm::vec3 pos, glm::vec3 force) {
		LOG("{} applying a force...", GetOwner()->GetName());
		//	Apply the force separately in x, y, z direction.

		for (int i = 0; i < 3; i++) {
			//	Cross product of force (i cross j).
			for (int j = 0; j < 3; j++) {
				//	Cross product case 1 - x cross x, y cross y, z cross z. Ignore.
				if (i == j) continue;

				//	Cross product case 2 - x cross y, y cross z, z cross x. Positive angular rotation. (if-case)
				//	Cross product case 3 - x cross z, y cross x, z cross y. Negative angular rotation. (else-case)
				if ((j + 3 - i) % 3 == 1) {
					m_angularAcceleration[(j + 1) % 3] += pos[j] * force[i];
				} else {
					m_angularAcceleration[(i + 1) % 3] -= pos[j] * force[i];
				}
			}
		}
		m_acceleration += pos * force / m_mass;
	}

	glm::vec3 Rigidbody::GetNormal(Rigidbody* other) const {
		glm::vec3 normal;

		glm::vec3 diff = other->transform->WorldPosition - transform->WorldPosition;

		//	Calculate the direction of the bounding box vectors.
		glm::vec3 xBound = glm::normalize(glm::vec3(1, 0, 0) * glm::conjugate(transform->LocalRotation));
		glm::vec3 yBound = glm::normalize(glm::vec3(0, 1, 0) * glm::conjugate(transform->LocalRotation));
		glm::vec3 zBound = glm::normalize(glm::vec3(0, 0, 1) * glm::conjugate(transform->LocalRotation));

		//	Break down the bounding box vectors.
		glm::vec3 xDiff = glm::dot(diff, xBound) * xBound / transform->LocalScale.x;
		glm::vec3 yDiff = glm::dot(diff, yBound) * yBound / transform->LocalScale.y;
		glm::vec3 zDiff = glm::dot(diff, zBound) * zBound / transform->LocalScale.z;

		//	Flip direction of vector if needed.
		if (glm::length(diff) * glm::length(xBound) / glm::dot(diff, xBound) < 0) xDiff *= -1;
		if (glm::length(diff) * glm::length(yBound) / glm::dot(diff, yBound) < 0) yDiff *= -1;
		if (glm::length(diff) * glm::length(zBound) / glm::dot(diff, zBound) < 0) zDiff *= -1;

		//	See which direction has the longest distance - the one with longest distance is the direction of normal.
		if (glm::length(xDiff) > glm::length(yDiff) && glm::length(xDiff) > glm::length(zDiff)) {
			normal = xDiff;
		} else if (glm::length(yDiff) > glm::length(xDiff) && glm::length(yDiff) > glm::length(zDiff)) {
			normal = yDiff;
		} else {
			normal = zDiff;
		}
		
		return glm::normalize(normal);
	}

	//void Rigidbody::Travel(float time, glm::vec3 linearAccel) {
	//	//	Damping algorithm (to be reviewed)
	//	if (glm::length(m_acceleration) > 0.0f) {
	//		m_acceleration *= (1 - m_linearDamping / 100.0f / glm::length(m_acceleration));
	//		linearAccel *= (1 - m_linearDamping / 100.0f / glm::length(m_acceleration));
	//	}

	//	float velocityConst = 1;
	//	if (glm::length(m_velocity) > 0.0f) {
	//		//m_velocity += linearAccel * time;
	//		velocityConst = (1 - RESISTANCE / glm::length(m_velocity));
	//	}

	//	transform->WorldPosition += m_velocity * time + 0.5f * linearAccel * time;
	//	m_velocity += linearAccel * time;
	//	m_velocity *= velocityConst;

	//	//	Damping algorithm (to be reviewed)
	//	if (glm::length(m_angularAcceleration) > 0.0f) {
	//		m_angularAcceleration *= (1 - m_linearDamping / 100.0f / glm::length(m_angularAcceleration));
	//	}

	//	if (glm::length(m_angularVelocity) > 0.0f) {
	//		m_angularVelocity *= (1 - RESISTANCE) / glm::length(m_angularVelocity);
	//	}

	//	transform->LocalEulerAngles += m_angularVelocity * time + 0.5f * m_angularAcceleration * time;
	//	m_angularVelocity += m_angularAcceleration * time;

	//	//	Map the rotation value back to (-180, 180].
	//	for (int i = 0; i < 3; i++) {
	//		if (transform->LocalEulerAngles[i] > 180) {
	//			transform->LocalEulerAngles[i] -= 360;
	//		} else if (transform->LocalEulerAngles[i] <= -180) {
	//			transform->LocalEulerAngles[i] += 360;
	//		}
	//	}
	//}

	//void Rigidbody::UndoTravel(float time, glm::vec3 linearAccel) {
	//	m_angularVelocity -= m_angularAcceleration * time;
	//	transform->LocalEulerAngles -= m_angularVelocity * time + 0.5f * m_angularAcceleration * time;

	//	m_velocity -= linearAccel * time;
	//	transform->WorldPosition -= m_velocity * time + 0.5f * linearAccel * time;
	//}

	void Rigidbody::Travel(float time, glm::vec3 linearAccel) {
		transform->WorldPosition += m_velocity * time + 0.5f * linearAccel * time;
		m_velocity += linearAccel * time;
		//	TODO: Damping algorithm
		m_velocity *= (100.0f - m_linearDamping) / 100.0f;

		transform->LocalEulerAngles += m_angularVelocity * time + 0.5f * m_angularAcceleration * time;
		m_angularVelocity += m_angularAcceleration * time;
		//	TODO: Damping algorithm
		m_angularVelocity *= (100.0f - m_angularDamping) / 100.0f;


		//LOG("Velocity ({}, {}, {})", m_velocity.x, m_velocity.y, m_velocity.z);
	}

	void Rigidbody::UndoTravel(float time, glm::vec3 linearAccel) {
		//	TODO: Damping algorithm
		//m_angularVelocity /= (100.0f - m_angularDamping) / 100.0f;
		m_angularVelocity -= m_angularAcceleration * time;
		transform->LocalEulerAngles -= m_angularVelocity * time + 0.5f * m_angularAcceleration * time;

		//	TODO: Damping algorithm
		//m_velocity /= (100.0f - m_linearDamping) / 100.0f;
		m_velocity -= linearAccel * time;
		transform->WorldPosition -= m_velocity * time + 0.5f * linearAccel * time;
	}
}