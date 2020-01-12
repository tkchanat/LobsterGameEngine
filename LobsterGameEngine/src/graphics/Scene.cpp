#include "pch.h"
#include "Scene.h"
#include "graphics/meshes/MeshFactory.h"
#include "graphics/Renderer.h"
#include "objects/GameObject.h"

namespace Lobster
{
    
    Scene::Scene(const char * scenePath) :
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

		// If scenePath is set, load and deserialize scene data
		if (scenePath[0] != '\0') {
			std::stringstream ss = FileSystem::ReadStringStream(FileSystem::Path(scenePath).c_str());
			Deserialize(ss);
		}
    }

	Scene::~Scene()
    {
		for (GameObject* gameObject : m_gameObjects)
		{
			if(gameObject)	delete gameObject;
			gameObject = nullptr;
		}
		if (m_skybox) delete m_skybox;
		m_skybox = nullptr;
    }
    
    void Scene::OnUpdate(double deltaTime)
    {
		Renderer::BeginScene(m_skybox);
        for(GameObject* gameObject : m_gameObjects)
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
		//std::vector<PhysicsComponent*> physics;

		/**
		 * Key: now every GameObject has PhysicsComponent. Just iterate every GameObjects.
		 */
		int i = 0;
		for (auto g1 : m_gameObjects) {
			//if (!(g1->GetComponent<PhysicsComponent>())) continue;
			
			//physics.push_back(g1->GetComponent<PhysicsComponent>());

			int j = 0;
			for (auto g2 : m_gameObjects) {
				if (i <= j) break;
				//if (!(g2->GetComponent<PhysicsComponent>())) continue;

				if (g1->Intersects(g2)) {
					g1->HasCollided(g2);
					g2->HasCollided(g1);
				}
				j++;
			}
			i++;
		}

		//	Finally, after detecting all collision on this frame -
		//	Time to update the physics. 
		//for (PhysicsComponent* physicsObj : physics) {
		for(auto go : m_gameObjects) {
			PhysicsComponent* physicsObj = go->GetComponent<PhysicsComponent>();
			if (!physicsObj) continue;
			physicsObj->OnPhysicsLateUpdate(deltaTime);
		}

		//	And eventually update the collision frame records.
		for (auto g1 : m_gameObjects) {
			g1->frameElapse();
		}
	}

	std::stringstream Scene::Serialize() {
		//LOG("Serializing Scene");
		std::stringstream ss;
		{
			cereal::JSONOutputArchive oarchive(ss);
			oarchive(*this);
		}
		INFO("Scene saved!");
		return ss;
	}

	void Scene::Deserialize(std::stringstream& ss) {
		//LOG("Deserializing Scene");
		cereal::JSONInputArchive iarchive(ss);
		try {
			iarchive(*this);
		}
		catch (std::exception e) {
			LOG("Deserializing Scene {} failed");
		}
	}
    
    Scene* Scene::AddGameObject(GameObject* gameObject)
    {
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
		if (!gameObject) return this;
		auto index = std::find(m_gameObjects.begin(), m_gameObjects.end(), gameObject);
		if (index != m_gameObjects.end()) {
			m_gameObjects.erase(index);
			//UndoSystem::GetInstance()->Push(new DestroyObjectCommand(gameObject, this));
			//delete gameObject;
			//gameObject = nullptr;
		}
		return this;
	}

	// Deprecated
	const std::vector<GameObject*>& Scene::GetGameObjects() {
		return m_gameObjects;
	}

	GameObject * Scene::GetGameObject(GameObject * gameObject)
	{
		std::stack<GameObject*> parents;
		GameObject* parent = gameObject->GetParent();
		while (parent != nullptr) {
			parents.push(parent);
			parent = parent->GetParent();
		}
		return nullptr;
	}
    
	bool Scene::IsObjectNameDuplicated(std::string name, std::string except) {
		for (auto& obj : m_gameObjects) {
			if (obj->GetName().compare(name) == 0 && obj->GetName().compare(except) != 0) return true;
		}
		return false;
	}
}
