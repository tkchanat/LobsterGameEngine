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
		// Note: no need for explicit release of memory due to smart pointers
		//for (GameObject* gameObject : m_gameObjects)
		//{
		//	if(gameObject)	delete gameObject;
		//	gameObject = nullptr;
		//}
    }
    
    void Scene::OnUpdate(double deltaTime)
    {
		Renderer::BeginScene(m_skybox);
        for(auto gameObject : m_gameObjects)
        {
            gameObject->OnUpdate(deltaTime);
        }
		Renderer::EndScene();
    }

	void Scene::OnPhysicsUpdate(double deltaTime) {
		//	First perform physics position update.
		for (auto gameObj : m_gameObjects) {
			PhysicsComponent* physicsObj = gameObj->GetComponent<PhysicsComponent>();
			if (physicsObj && physicsObj->IsEnabled()) physicsObj->OnPhysicsUpdate(deltaTime);
		}
		//	TODO: Some type of structure to record which pair of game objects / colliders intersected.

		//	Next, find the list of colliders and objects with physics component.
		//	This is done by finding all active rigidbody components.
		std::vector<PhysicsComponent*> physics;

		int i = 0;
		for (auto g1 : m_gameObjects) {
			if (!(g1->GetComponent<PhysicsComponent>())) continue;
			
			physics.push_back(g1->GetComponent<PhysicsComponent>());

			int j = 0;
			for (auto g2 : m_gameObjects) {
				if (i <= j) break;
				if (!(g2->GetComponent<PhysicsComponent>())) continue;

				if (g1->Intersects(g2.get())) {
					g1->HasCollided(g2.get());
				}
				j++;
			}
			i++;
		}

		//	Finally, after detecting all collision on this frame -
		//	Time to update the physics. 
		for (PhysicsComponent* physicsObj : physics) {
			physicsObj->OnPhysicsLateUpdate(deltaTime);
		}
	}

	//	TODO: @Yuki the starting point of the serialize function is here.
	//	You might need to create Serialize() and Deserialize() function for each member in Scene.
	//	You may want to debug by testing Serialize upon adding a game object.
	//	Simply copy this line and paste in AddGameObject function below:
	//	LOG("Serialization test result: {}", Serialize());
	//	
	//	TODO 2: Delete the TODO for this and the next function after completion :3

	//	Binary serialization of scene to prepare for saving.
	char* Scene::Serialize() const {
		//	1. Serialize header for scene

		//	2. Serialize content for scene, might involve recursive calls inside each member.

		//	3. Join each serialization result.
		//	A starting point could be creating a long-enough char *,
		//	then copy / concat using strcpy() and strcat() one-by-one.

		//	4. Return the result (and replace this dummy statement)
		return "Hello World!";
	}

	//	TODO: Deserialization testing a bit more difficult.
	//	Suggest to try after completing scene saving through serialization.
	void Scene::Deserialize(const char* serial) {
		//	1. Deserialize and assignment in Scene.

		//	2. Recurive call to assignment members
	}
    
    Scene* Scene::AddGameObject(GameObject* gameObject)
    {
        CameraComponent* camera = gameObject->GetComponent<CameraComponent>();
        if (camera) {
            if(m_activeCamera == nullptr)
				SetActiveCamera(camera);  //  Assign this to be main camera
            else
                LOG("Main camera for this scene has already been assigned. Ignoring this new camera...");
        }

		LightComponent* light = gameObject->GetComponent<LightComponent>();
		if (light) {
			LightLibrary::AddLight(light, light->GetType());
		}
        
        m_gameObjects.emplace_back(gameObject);

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
		if (!gameObject) return this;
		auto index = std::find(m_gameObjects.begin(), m_gameObjects.end(), std::shared_ptr<GameObject>(gameObject));
		if (index != m_gameObjects.end()) {
			m_gameObjects.erase(index);
			delete gameObject;
			gameObject = nullptr;
		}
		return this;
	}

	// Deprecated
	const std::vector<std::shared_ptr<GameObject>>& Scene::GetGameObjects() {
		return m_gameObjects;
	}
    
	bool Scene::IsObjectNameDuplicated(std::string name, std::string except) {
		for (auto& obj : m_gameObjects) {
			if (obj->GetName().compare(name) == 0 && obj->GetName().compare(except) != 0) return true;
		}
		return false;
	}
}
