#include "pch.h"
#include "physics/PhysicsComponent.h"
#include "objects/GameObject.h"

namespace Lobster {
	//	Thank you C++ for forcing me to create a cpp for these few line.
	//	We made const char* static members for ImGui dropdown box.
	//	If we found other workarounds, feel free to delete this line (and file).
	const char* PhysicsComponent::PhysicsBodyTypes[] = { "Rigid body", "Non-rigid body" };
	const char* PhysicsComponent::PhysicsType[] = { "Block", "Overlap", "Ignore" };

	void PhysicsComponent::RemoveCollider(Collider* collider) {
		int i = 0;
		for (Collider* c : m_colliders) {
			if (c == collider) m_colliders.erase(m_colliders.begin() + i);
			i++;
		}

		delete collider;
	}
}