#pragma once
#include "pch.h"
#include "objects/GameObject.h"

namespace Lobster
{

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
		case ComponentType::PARTICLE_COMPONENT:
			return new ParticleComponent();
		default:
			assert(false && "Please register your own typeName-to-component conversion here!");
			break;
		}
		return nullptr;
	}

}
