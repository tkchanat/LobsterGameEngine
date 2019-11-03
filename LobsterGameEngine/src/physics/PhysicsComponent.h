#pragma once
#include <vector>
#include "components/Component.h"
#include "physics/ColliderCollection.h"

namespace Lobster {
	class PhysicsComponent : public Component {
	public:
		static const char* PhysicsBodyTypes[];
		const static char* ColliderType[];
		const static char* PhysicsType[];

		PhysicsComponent(std::vector<glm::vec3> minMax) : m_minMax(minMax) {}

		//	inline int GetColliderType() const { return m_colliderType; }
		inline int GetPhysicsType() const { return m_physicsType; }
		inline Collider* GetCollider() const { return m_collider; }
		inline void SetEnabled() { ImGui::Checkbox("Enable physics?", &m_enabled); }

	protected:
		//	Bounding box is not visible to user, and is used by quick collision estimation only.
		Collider* m_boundingBox = nullptr;
		//	Collider is default to be equal to bounding box, except customizable.
		Collider* m_collider = nullptr;

		//	TODO: Temp and deletable
		std::vector<glm::vec3> m_minMax;
		//	Initialized to be of bound type.
		int m_physicsType = 0;
		int m_colliderType = 0;
	};
}