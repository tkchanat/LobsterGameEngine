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
		if (typeName == "class Lobster::MeshComponent" || typeName == "N7Lobster13MeshComponentE") {
			return new MeshComponent();
		}
		else if (typeName == "class Lobster::Rigidbody" || typeName == "N7Lobster9RigidbodyE") {
			return new Rigidbody();
		}
		else if (typeName == "class Lobster::CameraComponent" || typeName == "N7Lobster15CameraComponentE") {
			return new CameraComponent(ProjectionType::PERSPECTIVE);
		}
		else if (typeName == "class Lobster::LightComponent" || typeName == "N7Lobster14LightComponentE") {
			return new LightComponent(LightType::DIRECTIONAL_LIGHT);
		}
		else {
			throw std::runtime_error("Please register your own typeName-to-component conversion (" + typeName + ")");
			return nullptr;
		}
	}

}
