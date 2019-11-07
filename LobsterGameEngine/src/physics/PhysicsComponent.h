#pragma once
#include <vector>
#include "components/Component.h"
#include "physics/ColliderComponentCollection.h"

namespace Lobster {
	class PhysicsComponent : public Component {
	public:
		static const char* PhysicsBodyTypes[];
		const static char* PhysicsType[];

		~PhysicsComponent() {
			delete m_boundingBox;
			m_boundingBox = nullptr;
			for (ColliderComponent* collider : m_colliders) {
				delete collider;
			}
			m_colliders.clear();
		}

		void SetOwner(GameObject* owner);
		inline void SetOwnerTransform(Transform* t) { transform = t; m_boundingBox->SetOwnerTransform(t); }

		void AddCollider(ColliderComponent* collider) {
			m_colliders.push_back(collider);
		}
		inline std::vector<ColliderComponent*> GetColliders() const { return m_colliders; }
		//	Block / Overlap / Ignore
		inline int GetPhysicsType() const { return m_physicsType; }
		void RemoveCollider(ColliderComponent* collider);

	protected:
		//	Bounding box is not visible to user, and is used by quick collision estimation only.
		ColliderComponent* m_boundingBox = nullptr;

		//	Array of colliders.
		std::vector<ColliderComponent*> m_colliders;

		//	Initialized to be of bound type.
		int m_physicsType = 0;
	};
}