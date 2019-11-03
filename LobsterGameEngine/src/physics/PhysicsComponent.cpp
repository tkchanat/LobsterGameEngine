#include "pch.h"
#include "physics/PhysicsComponent.h"

namespace Lobster {
	//	Thank you C++ for forcing me to create a cpp for these few line.
	//	We made const char* static members for ImGui dropdown box.
	//	If we found other workarounds, feel free to delete this line (and file).
	const char* PhysicsComponent::PhysicsBodyTypes[] = { "Rigid body", "Non-rigid body" };

	const char* PhysicsComponent::ColliderType[] = { "Box Collider", "Sphere Collider" };
	const char* PhysicsComponent::PhysicsType[] = { "Bound", "Overlap", "Ignore" };
}