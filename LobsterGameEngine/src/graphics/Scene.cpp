#include "pch.h"
#include "Scene.h"
#include "graphics/meshes/MeshFactory.h"
#include "graphics/Renderer.h"
#include "graphics/Skybox.h"
#include "objects/GameObject.h"

namespace Lobster
{
    
    Scene::Scene(const char * scenePath) :
		m_skybox(nullptr)
    {
		// hard-coded skybox
		std::string faces[6] = { 
			FileSystem::Path("textures/skybox/px.png"),
			FileSystem::Path("textures/skybox/nx.png"),
			FileSystem::Path("textures/skybox/py.png"),
			FileSystem::Path("textures/skybox/ny.png"),
			FileSystem::Path("textures/skybox/pz.png"),
			FileSystem::Path("textures/skybox/nz.png") 
		};
		m_skybox = new Skybox(faces[0].c_str(), faces[1].c_str(), faces[2].c_str(), faces[3].c_str(), faces[4].c_str(), faces[5].c_str());

		// If scenePath is set, load and deserialize scene data
		if (scenePath && scenePath[0] != '\0') {
			std::stringstream ss = FileSystem::ReadStringStream(scenePath);
			Deserialize(ss);
		}
    }

	Scene::~Scene()
    {
		for (GameObject* gameObject : m_gameObjects) {
			if (gameObject)	delete gameObject;
			gameObject = nullptr;
		}
		if (m_skybox) delete m_skybox;
		m_skybox = nullptr;
    }

	void Scene::OnBegin() {
		for (GameObject* gameObject : m_gameObjects) {
			gameObject->OnBegin();
		}
	}
    
    void Scene::OnUpdate(double deltaTime)
    {
		Renderer::BeginScene(m_skybox->Get());
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

	void Scene::SetGameCamera(CameraComponent* camera) {
		m_gameCamera = camera;
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
			WARN("Deserializing Scene failed. Reason: {}", e.what());
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
    
	bool Scene::IsObjectNameDuplicated(std::string name, std::string except) {
		for (auto& obj : m_gameObjects) {
			if (obj->GetName().compare(name) == 0 && obj->GetName().compare(except) != 0) return true;
		}
		return false;
	}
}
