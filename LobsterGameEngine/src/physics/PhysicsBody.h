#pragma once
#include <vector>
#include "components/Component.h"
#include "physics/Collider.h"

namespace Lobster {
	class PhysicsBody : public Component {
	public:
		static const char* PhysicsBodyTypes[];

		PhysicsBody(std::vector<glm::vec3> minMax) : m_minMax(minMax) {}

		inline void SetOwner(GameObject* owner) {
			gameObject = owner;
			if (m_collider) m_collider->SetOwner(owner);
		}

		inline void SetOwnerTransform(Transform* t) {
			transform = t;
			if (m_collider) m_collider->SetOwnerTransform(t);
		}

		inline void RemoveOwner() {
			gameObject = nullptr;
			if (m_collider) m_collider->RemoveOwner();
		}

		inline Collider* GetCollider() const { return m_collider; }
		inline void SetEnabled() { ImGui::Checkbox("Enable physics?", &m_enabled); }

	protected:
		Collider* m_collider = nullptr;
		std::vector<glm::vec3> m_minMax;
	};
}