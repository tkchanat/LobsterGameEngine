#include "pch.h"
#include "Scene.h"
#include "graphics/meshes/MeshFactory.h"
#include "graphics/Renderer.h"
#include "objects/GameObject.h"

namespace Lobster
{
    
    Scene::Scene() :
		m_activeCamera(nullptr),
		m_skybox(nullptr)
    {
		// hard-coded skybox
		m_skybox = new TextureCube(
			"textures/skybox/px.png",
			"textures/skybox/nx.png",
			"textures/skybox/py.png",
			"textures/skybox/ny.png",
			"textures/skybox/pz.png",
			"textures/skybox/nz.png"
		);
    }
    
    Scene::~Scene()
    {
		for (GameObject* gameObject : m_gameObjects)
		{
			if(gameObject)	delete gameObject;
			gameObject = nullptr;
		}
    }
    
    void Scene::OnUpdate(double deltaTime)
    {
		Renderer::BeginScene(m_activeCamera, m_skybox);
        for(GameObject* gameObject : m_gameObjects)
        {
            gameObject->OnUpdate(deltaTime);
        }
		Renderer::EndScene();
    }

	void Scene::OnPhysicsUpdate(double deltaTime) {
		//	First perform physics update.
		//	TODO: Some type of structure to record which pair of game objects / colliders intersected.

		std::vector<ColliderComponent*> colliders;
		for (GameObject* gameObject : m_gameObjects) {
			//	TODO: Physics Update
			for (ColliderComponent* collider : gameObject->GetColliders()) {
				if (collider->GetOwner()->GetPhysicsComponent()->GetPhysicsType() != 2) colliders.push_back(collider);
			}
		}

		//	Next, do collision check on all physics components we extracted.
		//	Currently, we adapted a naive approach of comparing all pairs of AABB.
		int i = 0;
		for (ColliderComponent* c1 : colliders) {
			int j = 0;
			for (ColliderComponent* c2 : colliders) {
				if (i <= j) break;
				//	TODO: Actually use the computed result here instead of printing.
				bool intersect = c1->Intersects(c2);
				if (intersect) LOG("{} intersects with {} (Type: {})", c1->GetOwner()->GetName(), c2->GetOwner()->GetName(), PhysicsComponent::PhysicsType[std::max(c1->GetOwner()->GetPhysicsComponent()->GetPhysicsType(), c2->GetOwner()->GetPhysicsComponent()->GetPhysicsType())]);
				j++;
			}
			i++;
		}
	}
    
    Scene* Scene::AddGameObject(GameObject* gameObject)
    {
        CameraComponent* camera = gameObject->GetComponent<CameraComponent>();
        if(camera)
        {
            if(m_activeCamera == nullptr)
            {
				SetActiveCamera(camera);  //  Assign this to be main camera
            }
            else
            {
                LOG("Main camera for this scene has already been assigned. Ignoring this new camera...");
            }
        }
        
        m_gameObjects.push_back(gameObject);
        
        return this;
    }

	// Only remove the FIRST object with name
	Scene* Scene::RemoveGameObject(std::string name) {
		for (int i = 0; i < m_gameObjects.size(); i++) {
			if (m_gameObjects[i]->GetName().compare(name) == 0) {
				m_gameObjects.erase(m_gameObjects.begin() + i);
				break;
			}
		}
		return this;
	}

	Scene* Scene::RemoveGameObject(GameObject* gameObject) {
		std::remove(m_gameObjects.begin(), m_gameObjects.end(), gameObject);
		return this;
	}
    
	bool Scene::IsObjectNameDuplicated(std::string name, std::string except) {
		for (auto& obj : m_gameObjects) {
			if (obj->GetName().compare(name) == 0 && obj->GetName().compare(except) != 0) return true;
		}
		return false;
	}
}
