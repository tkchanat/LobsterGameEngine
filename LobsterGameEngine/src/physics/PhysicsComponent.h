#pragma once
#include <vector>
#include "components/Component.h"
#include "physics/ColliderCollection.h"

namespace Lobster {
	class PhysicsComponent : public Component {
	public:
		static const char* PhysicsBodyTypes[];
		const static char* PhysicsType[];

		~PhysicsComponent() {
			delete m_boundingBox;
			m_boundingBox = nullptr;
			for (Collider* collider : m_colliders) {
				delete collider;
			}
			m_colliders.clear();
		}

		void AddCollider(Collider* collider) {
			m_colliders.push_back(collider);
		}
		inline Collider* GetBoundingBox() const { return m_boundingBox; }
		inline std::vector<Collider*> GetColliders() const { return m_colliders; }

		bool Intersects(PhysicsComponent* other);
		virtual void OnPhysicsUpdate(double deltaTime) = 0;
		virtual void OnPhysicsLateUpdate(double deltaTime) = 0;

		//	Block / Overlap / Ignore
		inline int GetPhysicsType() const { return m_physicsType; }
		void RemoveCollider(Collider* collider);

	protected:
		//	Mass of component.
		float m_mass = 1.0f;

		bool m_simulate = false;

		//	Bounding box is not visible to user, and is used by quick collision estimation only.
		Collider* m_boundingBox = nullptr;

		//	Array of colliders.
		std::vector<Collider*> m_colliders;

		//	Initialized to be of bound type.
		int m_physicsType = 0;
	};
}