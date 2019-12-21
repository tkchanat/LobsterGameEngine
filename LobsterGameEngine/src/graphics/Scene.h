#pragma once

namespace Lobster
{
    
    class GameObject;
    class CameraComponent;
	class TextureCube;
	class LightComponent;

	//	This class encapsulates all the game objects and elements in the game world.
	//	A scene currently only allows one main camera, and a list of game objects.
	//	Ideally, game objects should form a hierarchy / tree. That means game objects can have children and parents, but the scene act as the root node of all.
    class Scene
    {
		friend class ImGuiHierarchy;
    private:
        CameraComponent* m_activeCamera; // non-removable main camera
		TextureCube* m_skybox;
        std::vector<GameObject*> m_gameObjects;
		std::vector<LightComponent*> m_lights;
    public:
        Scene();
        ~Scene();
        void OnUpdate(double deltaTime);
		void OnPhysicsUpdate(double deltaTime);
		char* Serialize() const;
		void Deserialize(const char* serial);
        Scene* AddGameObject(GameObject* gameObject);
		Scene* RemoveGameObject(std::string name);
		Scene* RemoveGameObject(GameObject* gameObject);
		const std::vector<GameObject*>& GetGameObjects();
		GameObject* GetGameObject(GameObject* gameObject);
		bool IsObjectNameDuplicated(std::string name, std::string except = "");
		inline void SetActiveCamera(CameraComponent* camera) { m_activeCamera = camera; }
        inline CameraComponent* GetActiveCamera() const { return m_activeCamera; }
    };
    
}
