#include "pch.h"
#include "physics/PhysicsComponent.h"
#include "objects/GameObject.h"

namespace Lobster {
	//	Thank you C++ for forcing me to create a cpp for these few line.
	//	We made const char* static members for ImGui dropdown box.
	//	If we found other workarounds, feel free to delete this line (and file).
	const char* PhysicsComponent::PhysicsBodyTypes[] = { "Rigid body", "Non-rigid body" };
	const char* PhysicsComponent::PhysicsType[] = { "Block", "Overlap", "Ignore" };

	void PhysicsComponent::SetOwner(GameObject* owner) {
		gameObject = owner;
		AABB* boundingBox = new AABB(Transform(), false);
		std::pair<glm::vec3, glm::vec3> pair = owner->GetComponent<MeshComponent>()->GetBound();
		boundingBox->Min = pair.first;
		boundingBox->Max = pair.second;
		boundingBox->SetOwner(gameObject);
		m_boundingBox = boundingBox;
	}

	void PhysicsComponent::RemoveCollider(ColliderComponent* collider) {
		int i = 0;
		for (ColliderComponent* c : m_colliders) {
			if (c == collider) m_colliders.erase(m_colliders.begin() + i);
			i++;
		}

		delete collider;
	}
}