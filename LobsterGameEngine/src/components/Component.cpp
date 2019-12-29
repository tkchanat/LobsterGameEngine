#pragma once
#include "pch.h"
#include "objects/GameObject.h"

namespace Lobster
{

	void Component::RemoveComponent(Component* comp) 
	{
		gameObject->RemoveComponent(comp);
	}

	Component* CreateComponentFromTypeName(const std::string& typeName, cereal::JSONInputArchive& iarchive)
	{
		if (typeName == "class Lobster::MeshComponent") {
			return new MeshComponent();
		}
		else if (typeName == "class Lobster::Rigidbody") {
			return new Rigidbody();
		}
		else if (typeName == "class Lobster::CameraComponent") {
			return new CameraComponent(ProjectionType::PERSPECTIVE);
		}
		else if (typeName == "class Lobster::LightComponent") {
			return new LightComponent(LightType::DIRECTIONAL_LIGHT);
		}
		else {
			throw std::runtime_error("Please register your own typeName-to-component conversion (" + typeName + ")");
			return nullptr;
		}
	}

}