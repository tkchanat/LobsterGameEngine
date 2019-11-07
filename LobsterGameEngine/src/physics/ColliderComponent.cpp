#pragma once
#include "pch.h"
#include "physics/ColliderComponent.h"
#include "objects/GameObject.h"

namespace Lobster {
	const char* ColliderComponent::ColliderType[] = { "Box Collider", "Sphere Collider" };

	void ColliderComponent::OnImGuiRender() {
		ImGui::PushID(this);
		if (ImGui::CollapsingHeader(ColliderType[m_colliderType], &m_show, ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Checkbox("Enabled?", &m_enabled);

			ImGui::Combo("Collider Type", &m_colliderType, ColliderType, 2);

			bool isChanging = false;
			ImGui::Indent();
			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::DragFloat3("Position", glm::value_ptr(m_transform.WorldPosition), 0.05f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
				isChanging = isChanging || ImGui::IsItemActive();
				//ImGui::DragFloat3("Rotation", glm::value_ptr(m_transform.LocalEulerAngles), 1.0f, -360.0f, 360.0f);
				//isChanging = isChanging || ImGui::IsItemActive();
				//ImGui::DragFloat3("Scale", glm::value_ptr(m_transform.LocalScale), 0.05f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
				//isChanging = isChanging || ImGui::IsItemActive();
			}
			ImGui::Unindent();

			//	TODO: When we add support for SphereCollider, add and implement this code to implement switching between Sphere / Box Collider.
			//	if (ImGui::IsItemActive());
		}
		ImGui::PopID();

		//	TODO: Confirmation Window.
		if (!m_show) {
			gameObject->GetComponent<PhysicsComponent>()->RemoveCollider(this);
		}
	}
}