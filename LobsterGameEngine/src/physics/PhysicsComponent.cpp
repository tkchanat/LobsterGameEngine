#include "pch.h"
#include "physics/PhysicsComponent.h"
#include "objects/GameObject.h"
#include "system/UndoSystem.h"

namespace Lobster {
	const char* PhysicsComponent::PhysicsBodyTypes[] = { "Rigid body", "Non-rigid body" };
	const char* PhysicsComponent::PhysicsType[] = { "Block", "Overlap", "Ignore" };

	PhysicsComponent::PhysicsComponent() : Component(PHYSICS_COMPONENT),
		m_velocity(glm::vec3(0, 0, 0)),
		m_acceleration(glm::vec3(0, 0, 0)),
		m_angularVelocity(glm::vec3(0, 0, 0)),
		m_angularAcceleration(glm::vec3(0, 0, 0))
	{
		PhysicsSystem::GetInstance()->AddPhysicsComp(this);
	}

	PhysicsComponent::~PhysicsComponent() {
		if (m_boundingBox) delete m_boundingBox;
		m_boundingBox = nullptr;
		for (Collider* collider : m_colliders) {
			if (collider) delete collider;
			collider = nullptr;
		}

		if (!b_isVirtuallyDeleted) PhysicsSystem::GetInstance()->RemovePhysicsComp(this);
	}

	void PhysicsComponent::VirtualCreate() {
		b_isVirtuallyDeleted = false;
		PhysicsSystem::GetInstance()->AddPhysicsComp(this);
	}

	void PhysicsComponent::VirtualDelete() {
		b_isVirtuallyDeleted = true;
		PhysicsSystem::GetInstance()->RemovePhysicsComp(this);
	}

	void PhysicsComponent::SetEnabledCallback() {
		UndoSystem::GetInstance()->Push(new PropertyAssignmentCommand(this, &m_enabled, !m_enabled, m_enabled, std::string(m_enabled ? "Enabled" : "Disabled") + " physics for " + GetOwner()->GetName()));
	}

	bool PhysicsComponent::Intersects(PhysicsComponent* other) {
		for (Collider* c1 : m_colliders) {
			if (c1->IsEnabled() == false) continue;
			for (Collider* c2 : other->m_colliders) {
				if (c2->IsEnabled() && c1->Intersects(c2)) return true;
			}
		}
		return false;
	}

	bool PhysicsComponent::OverlapTest(PhysicsComponent* other) {
		return m_physicsType == 1 || other->m_physicsType == 1;
	}

	void PhysicsComponent::Serialize(cereal::JSONOutputArchive & oarchive)
	{
	}

	void PhysicsComponent::Deserialize(cereal::JSONInputArchive & iarchive)
	{
	}

	void PhysicsComponent::RemoveCollider(Collider* collider) {
		int i = 0;
		for (Collider* c : m_colliders) {
			if (c == collider) m_colliders.erase(m_colliders.begin() + i);
			i++;
		}
	}
}