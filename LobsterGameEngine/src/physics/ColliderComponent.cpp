#pragma once
#include "pch.h"
#include "physics/ColliderComponent.h"

namespace Lobster {
	const char* ColliderComponent::ColliderType[] = { "Box Collider", "Sphere Collider" };

	void ColliderComponent::OnImGuiRender() {
		ImGui::PushID(this);
		if (ImGui::CollapsingHeader(ColliderType[m_colliderType], &m_show, ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Checkbox("Enabled?", &m_enabled);

			ImGui::Combo("Collider Type", &m_colliderType, ColliderType, 2);

			//	TODO: When we add support for SphereCollider, add and implement this code to implement switching between Sphere / Box Collider.
			//	if (ImGui::IsItemActive());
		}
		ImGui::PopID();

		//	TODO: Confirmation Window.
		if (!m_show) {
			RemoveComponent(this);
		}
	}
}