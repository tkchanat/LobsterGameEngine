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
		friend class ImGuiSkyboxEditor;
		friend class ImGuiScene;
		friend class ImGuiToolbar;
    private:
		Skybox* m_skybox;
        std::vector<GameObject*> m_gameObjects;
		std::string m_name;
    public:
        Scene(const char* scenePath = nullptr);
        ~Scene();
		void OnBegin();
        void OnUpdate(double deltaTime);
		void OnPhysicsUpdate(double deltaTime);
		std::stringstream Serialize();
		void Deserialize(std::stringstream& ss);
        Scene* AddGameObject(GameObject* gameObject);		
		Scene* RemoveGameObject(GameObject* gameObject);
		Scene* RemoveGameObjectByName(std::string name);
		bool IsObjectNameDuplicated(std::string name, std::string except = "");
		inline Skybox* GetSkybox() const { return m_skybox; }
	private:
		friend class cereal::access;
		template <class Archive>
		void save(Archive & ar) const
		{
			std::vector<std::string> childrenNames;
			for (auto child : m_gameObjects) childrenNames.push_back(child->GetName());
			ar(childrenNames);

			for (auto gameObject : m_gameObjects) {
				gameObject->Serialize(ar);
			}
		}
		template <class Archive>
		void load(Archive & ar)
		{
			std::vector<std::string> childrenNames;
			ar(childrenNames);
			for (auto name : childrenNames) AddGameObject(new GameObject(name.c_str()));

			for (auto gameObject : m_gameObjects) {
				gameObject->Deserialize(ar);
			}
		}
    };
    
}
