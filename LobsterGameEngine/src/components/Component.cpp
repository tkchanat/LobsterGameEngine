#pragma once
#include "pch.h"
#include "objects/GameObject.h"

namespace Lobster {
	void Component::RemoveComponent(Component* comp) {
		gameObject->RemoveComponent(comp);
	}
}