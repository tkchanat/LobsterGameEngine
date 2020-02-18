#include "pch.h"
#include "physics/PhysicsSystem.h"

namespace Lobster {
	PhysicsSystem* PhysicsSystem::m_instance = nullptr;

	PhysicsSystem::PhysicsSystem() {
		if (m_instance)
		{
			LOG("Physics System instance already created. Ignoring this ...");
			return;
		}
		m_instance = this;
	}
}