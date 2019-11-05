#pragma once
#include <vector>
#include "components/Component.h"
#include "physics/ColliderComponentCollection.h"

namespace Lobster {
	class PhysicsComponent : public Component {
	public:
		static const char* PhysicsBodyTypes[];
		const static char* PhysicsType[];

		void SetOwner(GameObject* owner);
		inline void SetOwnerTransform(Transform* t) { transform = t; m_boundingBox->SetOwnerTransform(t); }

		//	Block / Overlap / Ignore
		inline int GetPhysicsType() const { return m_physicsType; }

	protected:
		//	Bounding box is not visible to user, and is used by quick collision estimation only.
		ColliderComponent* m_boundingBox = nullptr;

		//	Initialized to be of bound type.
		int m_physicsType = 0;
	};
}