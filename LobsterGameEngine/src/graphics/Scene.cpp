#include "pch.h"
#include "Scene.h"
#include "graphics/meshes/MeshFactory.h"
#include "graphics/Renderer.h"
#include "objects/GameObject.h"

namespace Lobster
{
    
    Scene::Scene(const char * scenePath) :
		m_skybox(nullptr),
		m_physicsSystem(new PhysicsSystem())
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

		if (m_physicsSystem) delete m_physicsSystem;
		m_physicsSystem = nullptr;
    }

	void Scene::OnBegin() {
		for (GameObject* gameObject : m_gameObjects) {
			gameObject->OnBegin();
		}
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
	}

	std::stringstream Scene::Serialize() {
		//LOG("Serializing Scene");
		std::stringstream ss;
		{
			cereal::BinaryOutputArchive oarchive(ss);
			oarchive(*this);
		}
		INFO("Scene saved!");
		return ss;
	}

	void Scene::Deserialize(std::stringstream& ss) {
		//LOG("Deserializing Scene");
		cereal::BinaryInputArchive iarchive(ss);
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

	Scene* Scene::RemoveGameObject(GameObject* gameObject) {
		if (!gameObject) return this;
		auto index = std::find(m_gameObjects.begin(), m_gameObjects.end(), gameObject);
		if (index != m_gameObjects.end()) {
			m_gameObjects.erase(index);
		}
		return this;
	}

	// Only remove the FIRST object with name
	Scene* Scene::RemoveGameObjectByName(std::string name) {
		for (int i = 0; i < m_gameObjects.size(); i++) {
			if (m_gameObjects[i]->GetName() == name) {
				m_gameObjects.erase(m_gameObjects.begin() + i);
				break;
			}
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
