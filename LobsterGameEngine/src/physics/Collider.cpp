#pragma once
#include "pch.h"
#include "physics/Collider.h"
#include "objects/GameObject.h"
#include "physics/PhysicsComponent.h"
#include "system/UndoSystem.h"

namespace Lobster {
	const char* Collider::ColliderType[] = { "Axis-aligned Collider", "Box Collider", "Sphere Collider" };

	void Collider::OnImGuiRender() {
		bool changedColliderType = false;

		ImGui::PushID(this);
		if (ImGui::CollapsingHeader(ColliderType[m_colliderType], &m_show, ImGuiTreeNodeFlags_DefaultOpen)) {

			if (ImGui::Checkbox("Enabled?", &m_enabled)) {
				UndoSystem::GetInstance()->Push(new PropertyAssignmentCommand(this, &m_enabled, !m_enabled, m_enabled, std::string(m_enabled ? "Enabled" : "Unabled") + " a collider for " + physics->GetOwner()->GetName()));
			}

			m_prevColliderType = m_colliderType;
			if (ImGui::Combo("Collider Type", &m_colliderType, ColliderType, 3)) {
				if (m_prevColliderType != m_colliderType) {
					//	We will need to delete current object. Defer the change to the end of function.
					changedColliderType = true;
				}
			}

			ImGui::Indent();
			bool isChanging = false;
			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::DragFloat3("Position", glm::value_ptr(m_transform.WorldPosition), 0.05f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
				isChanging = isChanging || ImGui::IsItemActive();
				//	Only show rotation and scale option for OBB.
				if (m_colliderType == 1) {
					ImGui::DragFloat3("Rotation", glm::value_ptr(m_transform.LocalEulerAngles), 1.0f, -360.0f, 360.0f);
					isChanging = isChanging || ImGui::IsItemActive();
					ImGui::DragFloat3("Scale", glm::value_ptr(m_transform.LocalScale), 0.05f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
					isChanging = isChanging || ImGui::IsItemActive();
				}
			}
			ImGui::Unindent();

			//	Keep track of transform before we start transforming.
			if (!b_isChanging) {
				m_prevTransform = m_transform;
			}

			//	Now, keep track of whether we are changing, and send undo event after change.
			if (!b_isChanging && isChanging) {
				b_isChanging = true;
			} else if (b_isChanging && !isChanging) {
				b_isChanging = false;

				//	Only send undo event if transform changed.
				if (m_prevTransform.GetMatrix() != m_transform.GetMatrix()) {
					UndoSystem::GetInstance()->Push(new TransformColliderCommand(this, m_prevTransform, m_transform));
				}
			}
		}
		ImGui::PopID();

		//	TODO: Confirmation Window.
		if (!m_show) {
			physics->RemoveCollider(this);
			UndoSystem::GetInstance()->Push(new DestroyColliderCommand(this, physics));
			return;
		}

		if (changedColliderType) {
			Collider* newCollider = UpdateColliderType(m_colliderType);
			UndoSystem::GetInstance()->Push(new PropertyReplacementCommand(newCollider, &m_colliderType, m_prevColliderType, m_colliderType, "Set collider type to " + std::string(ColliderType[m_colliderType]) + " mode for " + physics->GetOwner()->GetName(), &Collider::UpdateColliderType));
			return;
		}
	}

	Collider* Collider::UpdateColliderType(int colliderType) {
		Collider* newCollider;

		//	TODO: When we add support for SphereCollider, add and implement this code to implement switching between Sphere / Box Collider.
		if (colliderType == 0) {
			newCollider = new AABB(physics);
		} else if (colliderType == 1) {
			newCollider = new BoxCollider(physics);
		}

		newCollider->SetOwner(physics->GetOwner());
		newCollider->SetOwnerTransform(transform);

		physics->AddCollider(newCollider);
		physics->RemoveCollider(this);
		delete this;

		return newCollider;
	}

	bool Collider::Intersects(Collider* c1, Collider* c2) {
		AABB* c1_AABB = dynamic_cast<AABB*>(c1);
		BoxCollider* c1_Box = dynamic_cast<BoxCollider*>(c1);

		AABB* c2_AABB = dynamic_cast<AABB*>(c2);
		BoxCollider* c2_Box = dynamic_cast<BoxCollider*>(c2);

		if (c1_AABB && c2_AABB) {
			bool x = (c1_AABB->Min.x + c1_AABB->Center.x - c2_AABB->Max.x - c2_AABB->Center.x) <= 0.0f && (c2_AABB->Min.x + c2_AABB->Center.x - c1_AABB->Max.x - c1_AABB->Center.x) <= 0.0f;
			bool y = (c1_AABB->Min.y + c1_AABB->Center.y - c2_AABB->Max.y - c2_AABB->Center.y) <= 0.0f && (c2_AABB->Min.y + c2_AABB->Center.y - c1_AABB->Max.y - c1_AABB->Center.y) <= 0.0f;
			bool z = (c1_AABB->Min.z + c1_AABB->Center.z - c2_AABB->Max.z - c2_AABB->Center.z) <= 0.0f && (c2_AABB->Min.z + c2_AABB->Center.z - c1_AABB->Max.z - c1_AABB->Center.z) <= 0.0f;

			return x && y && z;
		} else if (c1_Box && c2_AABB || c1_AABB && c2_Box || c1_Box && c2_Box) {
			//	We would need to compare 6 different axis. For each object, compare vec(B-A), vec(D-A), vec(E-A) with another object.
			std::vector<glm::vec3> p1 = c1->GetVertices();
			std::vector<glm::vec3> p2 = c2->GetVertices();

			return IsEnclosed(p1[0], p1[1], p2) && IsEnclosed(p1[0], p1[3], p2) && IsEnclosed(p1[0], p1[4], p2)
				&& IsEnclosed(p2[0], p2[1], p1) && IsEnclosed(p2[0], p2[3], p1) && IsEnclosed(p2[0], p2[4], p1);
		}
		return false;
	}

	bool Collider::IsEnclosed(glm::vec3 o1_min, glm::vec3 o1_max, std::vector<glm::vec3> o2) {
		//	First compute the vector direction.
		glm::vec3 projection = glm::normalize(o1_max - o1_min);

		float p2_min = 0.0f, p2_max = 0.0f;
		bool initialized = false;

		//	Now look for the minimum and maximum value of projected value for o2.
		//	i = the (i + 1)-th iteration. Always overwrite the value on the 1st iteration.
		for (glm::vec3 p : o2) {
			float projectedValue = glm::dot(p, projection);
			if (!initialized) {
				p2_min = p2_max = projectedValue;
				initialized = true;
			} else if (projectedValue < p2_min) {
				p2_min = projectedValue;
			} else if (projectedValue > p2_max) {
				p2_max = projectedValue;
			}
		}

		float p1_min = glm::dot(o1_min, projection);
		float p1_max = glm::dot(o1_max, projection);

		return p1_min <= p2_max && p2_min <= p1_max;
	}
}