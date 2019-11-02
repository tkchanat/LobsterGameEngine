#include "pch.h"
#include "physics/Collider.h"

namespace Lobster {
	Collider::Collider(std::vector<glm::vec3> minMax) :
		m_minMax(minMax)
	{

	}

	void Collider::OnUpdate(double deltaTime) {
		if (m_enabled) {
			m_boundingBox->OnUpdate(transform);
		}
	}

	void Collider::OnImGuiRender() {
		if (ImGui::CollapsingHeader("Collider", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Checkbox("Enable collision", &m_enabled);

			//	TODO: Bounding box type change
		}
	}
}