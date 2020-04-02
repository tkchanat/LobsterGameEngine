#pragma once
#include "objects/GameObject.h"

namespace Lobster
{

    class CameraComponent;
	class LightComponent;
	class Skybox;

	//	This class encapsulates all the game objects and elements in the game world.
	//	A scene currently only allows one main camera, and a list of game objects.
	//	Ideally, game objects should form a hierarchy / tree. That means game objects can have children and parents, but the scene act as the root node of all.
    class Scene
    {
		friend class ImGuiHierarchy;
		friend class ImGuiScene;
		friend class ImGuiToolbar;
    private:
		Skybox* m_skybox;
        std::vector<GameObject*> m_gameObjects;
		CameraComponent* m_gameCamera = nullptr; // a reference to game camera for easy access in script
		std::string m_name;
    public:
        Scene(const char* scenePath = nullptr);
        ~Scene();
		void OnBegin();
        void OnUpdate(double deltaTime);
		void OnPhysicsUpdate(double deltaTime);
		void SetGameCamera(CameraComponent* camera);
		std::stringstream Serialize();
		void Deserialize(std::stringstream& ss);
        Scene* AddGameObject(GameObject* gameObject);		
		Scene* RemoveGameObject(GameObject* gameObject);
		Scene* RemoveGameObjectByName(std::string name);
		bool IsObjectNameDuplicated(std::string name, std::string except = "");
		inline CameraComponent* GetGameCamera() const { return m_gameCamera; }
		inline Skybox* GetSkybox() const { return m_skybox; }
	private:
		friend class cereal::access;
		template <class Archive>
		void save(Archive & ar) const
		{
			// Scene Objects
			std::vector<std::string> childrenNames;
			for (auto child : m_gameObjects) childrenNames.push_back(child->GetName());
			ar(childrenNames);
			for (auto gameObject : m_gameObjects) {
				gameObject->Serialize(ar);
			}
			// Skybox
			m_skybox->Serialize(ar);
		}
		template <class Archive>
		void load(Archive & ar)
		{
			// Scene Objects
			std::vector<std::string> childrenNames;
			ar(childrenNames);
			for (auto name : childrenNames) AddGameObject(new GameObject(name.c_str()));
			for (auto gameObject : m_gameObjects) {
				gameObject->Deserialize(ar);
			}
			// Skybox
			m_skybox->Deserialize(ar);
		}
    };
    
}
