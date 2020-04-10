#include "pch.h"
#include "physics/PhysicsSystem.h"
#include "physics/PhysicsComponent.h"

namespace Lobster {
	PhysicsSystem* PhysicsSystem::m_instance = nullptr;

	void PhysicsSystem::Initialize()
	{
		if (m_instance) {
			WARN("Physics System instance already created. Ignoring this ...");
			return;
		}
		m_instance = new PhysicsSystem();
	}
}