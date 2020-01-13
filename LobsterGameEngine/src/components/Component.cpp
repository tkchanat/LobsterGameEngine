#pragma once
#include "pch.h"
#include "objects/GameObject.h"

namespace Lobster
{
	//	Maps a component enum to a string.
	std::string Component::componentName[] = {
		"Unknown",
		"Mesh Component",
		"Camera Component",
		"Light Component",
		"Physics Component",
		"Script Component",
		"Audio Source Component",
		"Audio Listener Component"
	};

	void Component::RemoveComponent(Component* comp) 
	{
		gameObject->RemoveComponent(comp);
	}

	Component* CreateComponentFromType(const ComponentType& type)
	{
		switch (type)
		{
		case ComponentType::MESH_COMPONENT:
			return new MeshComponent();
		case ComponentType::PHYSICS_COMPONENT:
			return nullptr; //new Rigidbody();
		case ComponentType::CAMERA_COMPONENT:
			return new CameraComponent();
		case ComponentType::LIGHT_COMPONENT:
			return new LightComponent(LightType::DIRECTIONAL_LIGHT);
		case ComponentType::AUDIO_SOURCE_COMPONENT:
			return new AudioSource();
		case ComponentType::AUDIO_LISTENER_COMPONENT:
			return new AudioListener();
		default:
			assert(false && "Please register your own typeName-to-component conversion here!");
			break;
		}
		return nullptr;
	}

}
