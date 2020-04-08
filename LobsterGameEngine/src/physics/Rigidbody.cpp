#include "pch.h"
#include "physics/Rigidbody.h"
#include "objects/GameObject.h"
#include "system/Input.h"
#include "system/UndoSystem.h"

namespace Lobster {
	//	Sunny: This value is open to change.
	const glm::vec3 Rigidbody::GRAVITY = glm::vec3(0, -0.0981, 0);
	const float Rigidbody::RESISTANCE = 0.001f;

	void Rigidbody::OnUpdate(double deltaTime) {
		//	We should update but not draw the bounding box. Update and draw the collider according to user's option.
		m_boundingBox->OnUpdate(deltaTime);
		for (Collider* collider : m_colliders) {
			if (collider->IsEnabled()) {
				collider->OnUpdate(deltaTime);
				if (Application::GetMode() != GAME) {
					collider->Draw();
				}				
			}
		}
	}

	Rigidbody::~Rigidbody()
	{
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
				if (ImGui::Checkbox("Enabled?", &m_enabled)) {
					UndoSystem::GetInstance()->Push(new PropertyAssignmentCommand(this, &m_enabled, !m_enabled, m_enabled, std::string(m_enabled ? "Enabled" : "Disabled") + " physics for " + GetOwner()->GetName()));
				}

				if (ImGui::Checkbox("Simulate Physics", &m_simulate)) {
					UndoSystem::GetInstance()->Push(new PropertyAssignmentCommand(this, &m_simulate, !m_simulate, m_simulate, std::string(m_simulate ? "Enabled" : "Disabled") + " physics simulation for " + GetOwner()->GetName()));
				}

				if (ImGui::DragFloat("Mass", &m_mass, 1.0f, 0.0001f, 100000.0f)) {
					m_isChanging = 0;
				}
				if (m_isChanging != 0) {
					m_prevProp[0] = m_mass;
				} else if (ImGui::IsItemActive() == false) {
					if (m_prevProp[0] != m_mass) {
						UndoSystem::GetInstance()->Push(new PropertyAssignmentCommand(this, &m_mass, m_prevProp[0], m_mass, "Set mass to " + StringOps::ToString(m_mass) + " for " + GetOwner()->GetName()));
					}
					m_isChanging = -1;
				}

				if (ImGui::SliderFloat("Linear Damping", &m_linearDamping, 0.0f, 100.0f)) {
					m_isChanging = 1;
				}
				if (m_isChanging != 1) {
					m_prevProp[1] = m_linearDamping;
				} else if (Input::IsMouseUp(GLFW_MOUSE_BUTTON_LEFT)) {
					if (m_prevProp[1] != m_linearDamping) {
						UndoSystem::GetInstance()->Push(new PropertyAssignmentCommand(this, &m_linearDamping, m_prevProp[1], m_linearDamping, "Set linear damping factor to " + StringOps::ToString(m_linearDamping, 2) + " for " + GetOwner()->GetName()));
					}
					m_isChanging = -1;
				}

				if (ImGui::SliderFloat("Angular Damping", &m_angularDamping, 0.0f, 100.0f)) {
					m_isChanging = 2;
				}
				if (m_isChanging != 2) {
					m_prevProp[2] = m_angularDamping;
				} else if (Input::IsMouseUp(GLFW_MOUSE_BUTTON_LEFT)) {
					if (m_prevProp[2] != m_angularDamping) {
						UndoSystem::GetInstance()->Push(new PropertyAssignmentCommand(this, &m_angularDamping, m_prevProp[2], m_angularDamping, "Set angular damping factor to " + StringOps::ToString(m_angularDamping, 2) + " for " + GetOwner()->GetName()));
					}
					m_isChanging = -1;
				}

				if (ImGui::SliderFloat("Elasticity", &m_restitution, 0.0f, 1.0f)) {
					m_isChanging = 3;
				}
				if (m_isChanging != 3) {
					m_prevProp[3] = m_restitution;
				} else if (Input::IsMouseUp(GLFW_MOUSE_BUTTON_LEFT)) {
					if (m_prevProp[3] != m_restitution) {
						UndoSystem::GetInstance()->Push(new PropertyAssignmentCommand(this, &m_restitution, m_prevProp[3], m_restitution, "Set elasticity factor to " + StringOps::ToString(m_restitution) + " for " + GetOwner()->GetName()));
					}
					m_isChanging = -1;
				}

				int prevPhysicsType = m_physicsType;
				if (ImGui::Combo("Physics Type", &m_physicsType, PhysicsType, 3)) {
					if (prevPhysicsType != m_physicsType) {
						UndoSystem::GetInstance()->Push(new PropertyAssignmentCommand(this, &m_physicsType, prevPhysicsType, m_physicsType, "Set physics type to " + std::string(PhysicsType[m_physicsType]) + " mode for " + GetOwner()->GetName()));
					}
				}
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
		double time = deltaTime / 1000;

		//	Find the actual colliding position.
		//	We do this by the method of trial and error -
		//	We will make 5 guesses, each with a timestep of (1/2)^(i)
		std::vector<PhysicsComponent*> collidingChecklist = PhysicsSystem::GetInstance()->compsList;

		//	We keep three extra variables out of the iteration loop -
		//	timestep will store the total time our object travelled.
		//	dt will store the deltaTime (in seconds) for each Travel or UndoTravel call.
		//	collidedObjects will store all objects with detected collision throughout the iterations.
		double timestep = 0;
		double dt = time;

		std::vector<PhysicsComponent*> collidedObjects;
		if (!m_simulate) return;

		for (int i = 0; i < 5; i++) {
			//	First, update the position of the object.
			Travel(dt);

			for (Collider* c : m_colliders) {
				c->OnUpdate(dt);
			}

			//	Next, determine if there exists any collision.
			bool hasCollided = false;
			for (PhysicsComponent* obj : collidingChecklist) {
				if (obj != this && Intersects(obj)) {
					hasCollided = true;

					if (std::find(collidedObjects.begin(), collidedObjects.end(), obj) == collidedObjects.end()) {
						collidedObjects.push_back(obj);
					}
				}
			}

			//	If there is collision, undo travel. Else, add dt to timestep.
			if (hasCollided) {
				UndoTravel(dt);
				//m_angularVelocity *= 0.8;
			} else {
				timestep += dt;
			}

			//	Finally, half the value of dt.
			dt /= 2;
		}
		m_acceleration = glm::vec3(0);

		//	After 5 iterations, time to resolve collisions (if any).
		//	If there are collisions, object travelled (ie: timestep != 0), need to resolve.
		//	Also keep track of center is out of object.
		bool centerOutOfObject = false;
		for (PhysicsComponent* obj : collidedObjects) {
			Rigidbody* other = dynamic_cast<Rigidbody*>(obj);
			if (!other) continue;

			//	1. Compute normal of collision.
			//	normal will store the normal pointing from other to this. (Used to resolve calculation)
			//	otherNormal will store the opposite normal. (Used to calculate angular motion)
			glm::vec3 normal = other->GetNormal(this);
			glm::vec3 otherNormal = GetNormal(other);
			float dotProduct = glm::dot(normal, otherNormal);

			if (timestep != 0) {
				//	2. Compute Impulse. We deferred the step of multiplying a constant to the next step.
				glm::vec3 impulse = -m_mass * other->m_mass * normal * glm::dot(normal, m_velocity - other->m_velocity);
				impulse /= (m_mass + other->m_mass);

				//	3. Apply angular motion due to lost of balance.
				//	Two cases to check -
				//	Case 1 - If the object's center's projection is inside another object.
				//	Case 2 - Not landing smoothly. (Only check if not satisfying Case 1)

				//	3.1 - Determine if the center of the object falls inside the opposing collider.
				glm::vec3 normalProjection = GetOwner()->transform.WorldPosition;
				glm::vec3 normNormal = glm::normalize(normal);
				normalProjection += glm::dot(obj->GetOwner()->transform.WorldPosition - GetOwner()->transform.WorldPosition, normNormal) * normNormal;

				//	3.1 - Try to map this object into another object's coordinate system.
				glm::vec3 mappedPosition = glm::inverse(obj->GetOwner()->transform.GetMatrix()) * glm::vec4(normalProjection, 1.0f);
				std::pair<glm::vec3, glm::vec3> pairBound = obj->GetOwner()->GetBound();

				//	3.1 - Ready to check if center is out of another object.
				//	displacement (glm::vec3) will record the distance from the boundary,
				//	in current object coordinate system, if it loses balance.
				//	Object need to travel in displacement direction to lose balance.
				//	centerOutOfObject (boolean) will record if it is outside another object.
				glm::vec3 displacement = glm::vec3(0, 0, 0);
				centerOutOfObject = false;

				//	Check if this object lies outside another obj by checking transformed bounding box.
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


				//	3.1 - Handle physics update if lost balance due to center out of object.
				if (centerOutOfObject) {
					//	1. Apply velocity to travel away.
					AddVelocity(displacement);
					AddVelocity(normal * glm::length(displacement));

					//	2. Rotation.
					glm::vec3 rotationVec = glm::cross(displacement, otherNormal);
					rotationVec *= 180 / M_PI;
					AddAngularVelocity(rotationVec);
				}

				//	3.2 - Check if object is not smoothly landing (ie: the two normals are not parallel)
				//	Since the normals are opposite in direction, their dot produict is expected to = -1.
				//	Rotational speed due unsmooth landing is assumed to be proportional to (180deg - angle).
				if (dotProduct != -1) {
					//	First determine direction of rotation.
					glm::vec3 direction = glm::cross(normal, otherNormal);

					//	Next compare with previous rotation.
					//	If the dot product <= 0, ie: larger than 90 degree angle change, flip the direction.
					bool flip = glm::dot(m_prevRotation, glm::normalize(direction)) <= 0;

					//	Calculate the direction of the bounding box vectors.
					glm::vec3 xBound = glm::normalize(glm::vec3(1, 0, 0) * glm::conjugate(transform->LocalRotation));
					glm::vec3 yBound = glm::normalize(glm::vec3(0, 1, 0) * glm::conjugate(transform->LocalRotation));
					glm::vec3 zBound = glm::normalize(glm::vec3(0, 0, 1) * glm::conjugate(transform->LocalRotation));

					//	Break down the direction into bounding box vectors (scalar).
					float xDiff = glm::dot(direction, xBound) / transform->LocalScale.x;
					float yDiff = glm::dot(direction, yBound) / transform->LocalScale.y;
					float zDiff = glm::dot(direction, zBound) / transform->LocalScale.z;
					float sumDiff = abs(xDiff) + abs(yDiff) + abs(zDiff);

					//	By theory sumDiff should never = 0. Add 1 to it to prevent DIV/0.
					if (sumDiff == 0) sumDiff = 1;

					//	Apply rotation.
					m_angularVelocity.x += xDiff / sumDiff * 180 / M_PI;
					m_angularVelocity.y += yDiff / sumDiff * 180 / M_PI;
					m_angularVelocity.z += zDiff / sumDiff * 180 / M_PI;

					if (flip) {
						if (m_angularVelocity.x * xDiff < 0) m_angularVelocity.x = (m_angularVelocity.x - xDiff / sumDiff * 10) * -0.8;
						if (m_angularVelocity.y * yDiff < 0) m_angularVelocity.y = (m_angularVelocity.y - yDiff / sumDiff * 10) * -0.8;
						if (m_angularVelocity.z * zDiff < 0) m_angularVelocity.z = (m_angularVelocity.z - zDiff / sumDiff * 10) * -0.8;
					}

					m_prevRotation = glm::normalize(direction);
				}

				//	4. Update velocity.
				//	Different impulse and speed depending on whether the two objects simulate physics.
				if (!(other->m_simulate)) {
					impulse *= (1 + m_restitution);
					m_velocity += impulse / m_mass + impulse / other->m_mass;
				} else {
					m_velocity += impulse * (1 + m_restitution) / m_mass;
					other->m_velocity -= impulse * (1 + other->m_restitution) / other->m_mass;
				}
			} else {
				//	If the object is not moving in this frame, we would reduce the angular velocity.
				//	This is a rough approximation of energy loss due to (air) friction.
				m_angularVelocity *= 0.9;
			}
		}

		//	Finally, let the game object travel for the remaining (timestep) amount of time it was supposed to travel.
		//	Only if it doesn't have angular velocity.
		float epsilon = 0.0001f;
		if (glm::length(m_angularVelocity) <= epsilon && !centerOutOfObject)
		Travel(time - timestep, timestep != 0, false);

		//	Invoke functions for collision handling.
		//	If previous frame has no collision, call the two functions.
		for (PhysicsComponent* obj : collidedObjects) {
			if (std::find(m_prevCollidingList.begin(), m_prevCollidingList.end(), obj) == m_prevCollidingList.end()) {
				gameObject->OnEnter(obj->GetOwner());
				obj->GetOwner()->OnEnter(gameObject);

				gameObject->OnCollide(obj->GetOwner());
				obj->GetOwner()->OnCollide(gameObject);
			}
		}

		//	Call OnLeave if objects are not colliding at this frame, otherwise call OnOverlap.
		for (PhysicsComponent* obj : m_prevCollidingList) {
			if (std::find(collidedObjects.begin(), collidedObjects.end(), obj) == collidedObjects.end()) {
				gameObject->OnLeave(obj->GetOwner());
				obj->GetOwner()->OnLeave(gameObject);
			} else {
				gameObject->OnOverlap(obj->GetOwner());
				obj->GetOwner()->OnOverlap(gameObject);
			}
		}

		//	Finally, update list of previously colliding objects.
		m_prevCollidingList = collidedObjects;

		//	Fix angles. Angle range is (-180, 180).
		for (int i = 0; i < 3; i++) {
			while (transform->LocalEulerAngles[i] > 180.0f) transform->LocalEulerAngles[i] -= 360.0f;
			while (transform->LocalEulerAngles[i] <= -180.0f) transform->LocalEulerAngles[i] += 360.0f;
		}
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

	void Rigidbody::Travel(float time, bool gravity, bool damping) {
		//	First, we update the position here.
		if (gravity) {
			glm::vec3 newAcceleration = m_acceleration + GRAVITY;
			glm::vec3 newVelocity = m_velocity + newAcceleration * time;
			transform->WorldPosition += newVelocity * time;
			m_velocity = newVelocity;
			//transform->WorldPosition += m_velocity * time + 0.5f * m_acceleration * time;
			//transform->WorldPosition += 0.5f * GRAVITY * time;
			//m_velocity += (m_acceleration + GRAVITY) * time;
		} else {
			glm::vec3 velocity = glm::vec3(m_velocity.x, 0, m_velocity.z);
			glm::vec3 accel = glm::vec3(m_acceleration.y, 0, m_acceleration.z);
			transform->WorldPosition += velocity *time + 0.5f * accel * time;
			m_velocity += m_acceleration * time;
		}

		//	TODO: Damping algorithm fix
		//	Next, after updating the position, we update the acceleration by damping formula.
		if (damping) {
			m_acceleration *= pow(1.0f - m_linearDamping / 100.0f, time);
		}
		//m_velocity *= (100.0f - m_linearDamping) / 100.0f;

		//	Same applies for angular position.
		transform->LocalEulerAngles += m_angularVelocity * time + 0.5f * m_angularAcceleration * time;
		m_angularVelocity += m_angularAcceleration * time;

		//	TODO: Damping algorithm fix
		//	Finally, update angular acceleration by damping formula.
		if (damping) {
			m_angularAcceleration *= pow(1.0f - m_angularDamping / 100.0f, time);
		}
	}

	void Rigidbody::UndoTravel(float time) {
		//	TODO: Damping algorithm fix
		//	1. Undo angular acceleration damping.
		m_angularAcceleration /= pow(1.0f - m_angularDamping / 100.0f, time);

		//	2. Undo angular position update.
		m_angularVelocity -= m_angularAcceleration * time;
		transform->LocalEulerAngles -= m_angularVelocity * time + 0.5f * m_angularAcceleration * time;

		//	TODO: Damping algorithm
		//	3. Undo acceleration damping.
		m_acceleration /= pow(1.0f - m_linearDamping / 100.0f, time);

		//	4. Undo position update.
		glm::vec3 newAcceleration = m_acceleration + GRAVITY;
		glm::vec3 newVelocity = m_velocity + newAcceleration * time;
		transform->WorldPosition -= newVelocity * time;
		m_velocity = newVelocity - newAcceleration * time;
		//m_velocity -= ( m_acceleration + GRAVITY) * time;
		//transform->WorldPosition -= 0.5f * GRAVITY * time;
		//transform->WorldPosition -= m_velocity * time + 0.5f * m_acceleration * time;
	}
}