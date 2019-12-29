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
		//	First perform physics update.
		//	TODO: Some type of structure to record which pair of game objects / colliders intersected.

		std::vector<Collider*> colliders;
		for (GameObject* gameObject : m_gameObjects) {
			//	TODO: Physics Update
			Rigidbody* rigidbody = gameObject->GetComponent<Rigidbody>();
			if (!rigidbody || !rigidbody->IsEnabled()) continue;
			for (Collider* collider : rigidbody->GetColliders()) {
				if (collider->IsEnabled() && collider->GetPhysics()->GetPhysicsType() != 2)
					colliders.push_back(collider);
			}
		}

		//	Next, do collision check on all physics components we extracted.
		//	Currently, we adapted a naive approach of comparing all pairs of AABB.
		int i = 0;
		for (Collider* c1 : colliders) {
			int j = 0;
			for (Collider* c2 : colliders) {
				if (i <= j) break;
				//	TODO: Actually use the computed result here instead of printing.
				bool intersect = c1->Intersects(c2) && (c1->GetPhysics() != c2->GetPhysics());
				if (intersect) LOG("{} intersects with {} (Type: {})", c1->GetPhysics()->GetOwner()->GetName(), c2->GetPhysics()->GetOwner()->GetName(), PhysicsComponent::PhysicsType[std::max(c1->GetPhysics()->GetPhysicsType(), c2->GetPhysics()->GetPhysicsType())]);
				j++;
			}
			i++;
		}
	}

	std::stringstream Scene::Serialize() {
		LOG("Serializing Scene");
		std::stringstream ss;
		{
			cereal::JSONOutputArchive oarchive(ss);
			oarchive(*this);
		}
		return ss;
	}

	void Scene::Deserialize(std::stringstream& ss) {
		LOG("Deserializing Scene");
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
			delete gameObject;
			gameObject = nullptr;
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
