#include "pch.h"
#include "physics/PhysicsComponent.h"
#include "objects/GameObject.h"

namespace Lobster {
	//	Thank you C++ for forcing me to create a cpp for these few line.
	//	We made const char* static members for ImGui dropdown box.
	//	If we found other workarounds, feel free to delete this line (and file).
	const char* PhysicsComponent::PhysicsBodyTypes[] = { "Rigid body", "Non-rigid body" };
	const char* PhysicsComponent::PhysicsType[] = { "Block", "Overlap", "Ignore" };

	bool PhysicsComponent::Intersects(PhysicsComponent* other) {
		for (Collider* c1 : m_colliders) {
			if (c1->IsEnabled() == false) continue;
			for (Collider* c2 : other->m_colliders) {
				if (c2->IsEnabled() && c1->Intersects(c2)) return true;
			}
		}
		return false;
	}

	void PhysicsComponent::Serialize(cereal::JSONOutputArchive & oarchive)
	{
		oarchive(*this);
	}

	void PhysicsComponent::Deserialize(cereal::JSONInputArchive & iarchive)
	{
		try {
			iarchive(*this);
		}
		catch (std::exception e) {
			LOG("Deserializing PhysicsComponent failed. Reason: {}", e.what());
		}
	}

	void PhysicsComponent::RemoveCollider(Collider* collider) {
		int i = 0;
		for (Collider* c : m_colliders) {
			if (c == collider) m_colliders.erase(m_colliders.begin() + i);
			i++;
		}

		delete collider;
	}
}